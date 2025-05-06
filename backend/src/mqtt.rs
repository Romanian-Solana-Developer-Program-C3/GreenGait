use rumqttc::tokio_rustls::rustls::{
    Certificate, ClientConfig as RustlsClientConfig, PrivateKey, RootCertStore,
};
use rumqttc::{AsyncClient, Event, Incoming, MqttOptions, TlsConfiguration, Transport};
use rustls_pemfile::{certs, pkcs8_private_keys};
use std::fs::File;
use std::io::BufReader;
use std::sync::Arc;
use std::time::Duration;
use tokio::task::spawn_blocking;

use crate::blockchain::log_step_on_chain;
use crate::config::*;
use crate::security::{clean_payload, verify_hmac, verify_timestamp};
use crate::state::STATUS;

pub async fn start_mqtt() {
    // 🔐 1. TLX mutual certificates reading
    let mut ca_reader = BufReader::new(File::open(CA_CERT).expect("Cannot open CA cert"));
    let ca_certs = certs(&mut ca_reader).expect("Cannot read CA certs");

    let mut client_cert_reader =
        BufReader::new(File::open(CLIENT_CERT).expect("Cannot open client cert"));
    let client_certs = certs(&mut client_cert_reader).expect("Cannot read client certs");

    let mut client_key_reader =
        BufReader::new(File::open(CLIENT_KEY).expect("Cannot open client key"));
    let client_keys = pkcs8_private_keys(&mut client_key_reader).expect("Cannot read client key");

    let mut root_cert_store = RootCertStore::empty();
    for cert in ca_certs {
        root_cert_store.add(&Certificate(cert)).unwrap();
    }

    let rustls_config = RustlsClientConfig::builder()
        .with_safe_defaults()
        .with_root_certificates(root_cert_store)
        .with_single_cert(
            client_certs.into_iter().map(Certificate).collect(),
            PrivateKey(client_keys[0].clone()),
        )
        .expect("Rustls config failed");

    // 🔌 2. Configuration MQTT
    let mut mqttoptions = MqttOptions::new(CLIENT_ID, MQTT_BROKER, MQTT_PORT);
    mqttoptions.set_keep_alive(Duration::from_secs(5));
    mqttoptions.set_transport(Transport::tls_with_config(TlsConfiguration::Rustls(
        Arc::new(rustls_config),
    )));

    let (client, mut eventloop) = AsyncClient::new(mqttoptions, 10);
    client
        .subscribe(MQTT_TOPIC, rumqttc::QoS::AtLeastOnce)
        .await
        .unwrap();

    println!("[MQTT] Subscribed to topic: {}", MQTT_TOPIC);

    // 🔁 3. Listen the received messages
    loop {
        let event = eventloop.poll().await;
        if let Ok(Event::Incoming(Incoming::Publish(p))) = event {
            let payload_str = String::from_utf8_lossy(&p.payload);
            println!("[MQTT] Received: {}", payload_str);

            if let Ok(json) = serde_json::from_str::<serde_json::Value>(&payload_str) {
                if let (Some(steps), Some(timestamp), Some(_nonce), Some(signature)) = (
                    json.get("steps"),
                    json.get("timestamp"),
                    json.get("nonce"),
                    json.get("signature"),
                ) {
                    let clean = clean_payload(&payload_str);
                    if verify_hmac(&clean, signature.as_str().unwrap(), HMAC_SECRET) {
                        if verify_timestamp(timestamp.as_i64().unwrap()) {
                            println!("[SECURITY] ✅ Valid HMAC and Timestamp - Steps: {}", steps);

                            // 🔗 4. Save the steps on blockchain
                            let user_pubkey =
                                json.get("pubkey").unwrap().as_str().unwrap().to_string();

                            let steps = steps.as_u64().unwrap();

                            use chrono::Utc;
                            let now = Utc::now().naive_utc();
                            let day = now.format("%Y%m%d").to_string().parse::<i64>().unwrap();

                            spawn_blocking(move || {
                                let mint_address = "2S17Ma6eDo2NgZQDv6Vda3hKJPwaCtBWqhen5ThVU3yk";
                                let result =
                                    tokio::runtime::Handle::current().block_on(async move {
                                        log_step_on_chain(&user_pubkey, steps, day, mint_address)
                                            .await
                                    });

                                match result {
                                    Ok(sig) => {
                                        println!("[CHAIN] ✅ Step logged on Solana - Tx: {}", sig);
                                        // ✅ Synchronize STATUS global
                                        let mut status = STATUS.lock().unwrap();
                                        status.steps += steps;
                                        status.tokens = (status.steps as f64) / 3.0;
                                    }
                                    Err(e) => eprintln!("[CHAIN] ❌ Error logging on chain: {}", e),
                                }
                            });
                        } else {
                            println!("[SECURITY] ❌ Invalid Timestamp");
                        }
                    } else {
                        println!("[SECURITY] ❌ Invalid HMAC");
                    }
                } else {
                    println!("[SECURITY] ❌ Invalid payload structure");
                }
            }
        }
    }
}
