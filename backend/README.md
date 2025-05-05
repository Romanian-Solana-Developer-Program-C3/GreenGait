# 🍃 GreenGait – Step Into Web3 Rewards 🍃

**GreenGait** is a Web3 rewards app that turns your steps into real value using blockchain technology. With a wearable shoe-mounted device, every step is securely tracked, cryptographically signed, and sent to a backend that logs it on the Solana blockchain. 🚶‍♂️→ 💰

## 🌐 Architecture Overview

🟢 **ESP32-WROOM-32D** – The main microcontroller, connected via Wi-Fi, with a physical button used to simulate walking steps.

🔒 **TLS Mutual Authentication** – Secure communication between the ESP32 device and a cloud platform using MQTT over TLS (port 8883) with client certificate authentication.

☁️ **Google Cloud VPS + EMQX** – A secure MQTT broker hosted on a GCP virtual private server, configured with mutual TLS and ACL rules.

🧠 **Rust Backend** – A validator application that receives messages via MQTT, verifies HMAC signatures and timestamps, and logs step data to the blockchain.

📦 **Blockchain Integration (WIP)** – Solana/Anchor integration is in progress. For now, symbolic transactions (e.g. 0.00001 SOL) are sent for validation purposes.

---

## ✅ Features Implemented So Far

- ✅ ESP32 connected to WiFi with mutual TLS authentication
- ✅ HMAC signature generation on the device (`crypto_hmac.h`)
- ✅ JSON payload transmission including `steps`, `timestamp`, `nonce`, and `signature`
- ✅ Rust backend:
  - TLS-authenticated MQTT client
  - HMAC and timestamp validation
  - Step data logging via symbolic Solana transaction (`blockchain.rs`)
- ✅ Google Cloud VPS running secure EMQX broker

---

## 🔐 Security Design

- HMAC-SHA256 authentication with a pre-shared secret
- Timestamp validation to prevent replay attacks (±30 seconds)
- Mutual TLS (device ↔ broker)
- Backend isolated on a hardened cloud VPS

---

## 📁 Project Structure

firmware/ ├── ESP32.ino # ESP32 code (WiFi + MQTT + HMAC) ├── certificates.h # CA certificate, client cert & key ├── crypto_hmac.h # HMAC-SHA256 function

backend/ ├── main.rs # Rust backend entry point ├── mqtt.rs # TLS MQTT client + message handling ├── blockchain.rs # Solana step logging (symbolic tx) ├── config.rs # Broker config + certificate paths ├── security.rs # HMAC and timestamp validation


---

## 🛠️ What's Next?

- [ ] 🪙 Full Solana Anchor program integration
- [ ] 🧠 PDAs per user & persistent on-chain step accounts
- [ ] 🎨 UI for step history and rewards display
- [ ] 💎 Token/NFT minting as walking rewards

---

## 🚀 How to Run

### 1. Flash the ESP32
Use Arduino IDE or PlatformIO to upload `ESP32.ino`. Make sure `certificates.h` includes the TLS certificates generated for EMQX.

### 2. Run the Rust Backend

```bash
cd backend
cargo run
```
Ensure the following certificate/key files exist under backend/certs/:

```bash
ca.crt
client.crt
client.key
stepmint-validator.json (Solana keypair)
```

---

## 📬 Contact
Robert Panța

MSc Student in Cybersecurity – Technical University of Cluj-Napoca

📫 **[LinkedIn](https://www.linkedin.com/in/robert-panta/)**
🌐 **[GitHub](https://github.com/RobCyberLab)**