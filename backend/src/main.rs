mod blockchain;
mod config;
mod mqtt;
mod security;
mod state;

use crate::state::{StepInfo, STATUS};
use axum::{routing::get, Json, Router};
use std::net::SocketAddr;
use tower_http::services::ServeDir;
use serde::Deserialize;
use reqwest::Client;



async fn get_status() -> Json<StepInfo> {
    let status = STATUS.lock().unwrap().clone();
    let steps = status.steps;

    let ata_pubkey_str = "RfBDDpuQLd4KV2AyZtmN1Ti5SxAHjuGTpMdmx8q2UVh";
    let token_amount = fetch_token_balance(ata_pubkey_str).await;

    Json(StepInfo {
        steps,
        tokens: token_amount,
    })
}


#[derive(Deserialize)]
struct TokenAmount {
    #[serde(rename = "uiAmount")]
    ui_amount: Option<f64>,
}

#[derive(Deserialize)]
struct RpcResponse {
    result: Option<RpcResult>,
}

#[derive(Deserialize)]
struct RpcResult {
    value: TokenAmount,
}

async fn fetch_token_balance(ata: &str) -> f64 {
    let client = Client::new();
    let rpc_url = "https://api.devnet.solana.com";

    let payload = serde_json::json!({
        "jsonrpc": "2.0",
        "id": 1,
        "method": "getTokenAccountBalance",
        "params": [ata]
    });

    match client.post(rpc_url).json(&payload).send().await {
        Ok(res) => match res.json::<RpcResponse>().await {
            Ok(parsed) => parsed.result
                .and_then(|r| r.value.ui_amount)
                .unwrap_or(0.0),
            Err(e) => {
                eprintln!("❌ Parse error: {e}");
                0.0
            }
        },
        Err(e) => {
            eprintln!("❌ Request error: {e}");
            0.0
        }
    }
}


#[tokio::main]
async fn main() {
    println!("[SYSTEM] GreenGait Backend Validator Starting...");

    // 🧠 Runs Web UI + REST API on a searate thread
    tokio::spawn(async {
        let app = Router::new()
            .route("/status", get(get_status)) // Endpoint API
            .nest_service(
                "/",
                axum::routing::get_service(ServeDir::new("../frontend")),
            ); // UI static

        let addr = SocketAddr::from(([0, 0, 0, 0], 3000));
        println!("🌍 Web dashboard available at: http://localhost:3000");
        let listener = tokio::net::TcpListener::bind(addr).await.unwrap();
        axum::serve(listener, app).await.unwrap();
    });

    // 🔗 Listen MQTT for ESP32 in main thread
    mqtt::start_mqtt().await;
}







// mod blockchain;
// mod config;
// mod mqtt;
// mod security;
// mod state;

// use crate::state::{StepInfo, STATUS};
// use axum::{routing::get, Json, Router};
// use solana_client::rpc_client::RpcClient;
// use solana_sdk::pubkey::Pubkey;
// use std::net::SocketAddr;
// use tower_http::services::ServeDir;

// async fn get_status() -> Json<StepInfo> {
//     let status = STATUS.lock().unwrap().clone();
//     let steps = status.steps;

//     // 🔗 Solana Devnet RPC
//     let rpc_url = "https://api.devnet.solana.com";
//     let client = RpcClient::new(rpc_url.to_string());

//     // ✅ Replace this with dynamic ATA in the future
//     let ata_pubkey_str = "RfBDDpuQLd4KV2AyZtmN1Ti5SxAHjuGTpMdmx8q2UVh";
//     let ata_pubkey = ata_pubkey_str.parse::<Pubkey>().unwrap();

//     // 🔄 Get token amount from blockchain
//     let token_amount: f64 = match client.get_token_account_balance(&ata_pubkey) {
//         Ok(result) => result.ui_amount.unwrap_or(0.0),
//         Err(e) => {
//             eprintln!("❌ Error getting token balance: {e}");
//             0.0
//         }
//     };
//     Json(StepInfo {
//         steps,
//         tokens: token_amount,
//     })
// }

// #[tokio::main]
// async fn main() {
//     println!("[SYSTEM] GreenGait Backend Validator Starting...");

//     // 🧠 Runs Web UI + REST API on a searate thread
//     tokio::spawn(async {
//         let app = Router::new()
//             .route("/status", get(get_status)) // Endpoint API
//             .nest_service(
//                 "/",
//                 axum::routing::get_service(ServeDir::new("../frontend")),
//             ); // UI static

//         let addr = SocketAddr::from(([0, 0, 0, 0], 3000));
//         println!("🌍 Web dashboard available at: http://localhost:3000");
//         let listener = tokio::net::TcpListener::bind(addr).await.unwrap();
//         axum::serve(listener, app).await.unwrap();
//     });

//     // 🔗 Listen MQTT for ESP32 in main thread
//     mqtt::start_mqtt().await;
// }
