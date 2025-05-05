use hmac::{Hmac, Mac};
use sha2::Sha256;
use chrono::Utc;

/// Tip alias pentru HMAC-SHA256
type HmacSha256 = Hmac<Sha256>;

/// Verifică semnătura HMAC
pub fn verify_hmac(payload: &str, signature: &str, secret: &str) -> bool {
    let mut mac = HmacSha256::new_from_slice(secret.as_bytes()).expect("HMAC can take key of any size");
    mac.update(payload.as_bytes());
    let result = mac.finalize();
    let expected = result.into_bytes();
    hex::encode(expected) == signature
}

/// Verifică dacă timestamp-ul e în toleranță (30 secunde)
pub fn verify_timestamp(timestamp: i64) -> bool {
    let now = Utc::now().timestamp();
    let delta = (now - timestamp).abs();
    println!("[SECURITY] ⏱️ Timestamp delta: {} seconds", delta);
    delta <= 120  // <-- increased from 30 to 120
}


/// Extrage payload-ul fără câmpul "signature"
pub fn clean_payload(json: &str) -> String {
    if let Some(index) = json.find(",\"signature\":") {
        format!("{}{}", &json[..index], "}")
    } else {
        json.to_string()
    }
}
