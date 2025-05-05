# 🍃 GreenGait 🍃

**GreenGait** is a Web3 rewards platform that transforms physical activity into real digital value using the Solana blockchain. With a secure, Wi-Fi-enabled wearable device (ESP32), every step you take is cryptographically signed and submitted to the blockchain – all in real time.
🏃 + ✅ → 💰 on-chain.

---

## Table of Contents 📁

1. [**Architecture Overview**](#architecture-overview-)
2. [**Features Implemented**](#features-implemented-)
3. [**Security Architecture**](#security-architecture-)
4. [**Project Structure**](#project-structure)
5. [**Example Flow**](#example-flow)
6. [**How to Run Locally**](#how-to-run-locally)
7. [**What's Next?**](#whats-next)
8. [**Live Preview**](#live-preview)
9. [**Author**](#author)

---

## 🌐 Architecture Overview

🔹 **ESP32-WROOM-32D**
A microcontroller that simulates steps via a button. Sends step data via MQTT over TLS using mutual certificate authentication.

🔐 **TLS Mutual Authentication**
Secure communication using custom client certificates and a trusted CA, protecting against unauthorized devices.

🧠 **Rust Backend Validator**
Receives messages via MQTT, validates authenticity (HMAC + timestamp), then logs valid steps on-chain via a Solana smart contract.

⛓️ **Solana Anchor Program**
Deployed on Devnet. Uses Program Derived Addresses (PDAs) to store step data per user per day. Tokens are minted automatically every 3 steps.

📡 **EMQX Broker (Google Cloud VPS)**
A hardened MQTT broker with TLS, ACL rules, and certificate-based access control.

🖥️ **Frontend Interface** *(in progress)*
Visualizes your step history and blockchain rewards in a simple dashboard.

---

## ✅ Features Implemented

* ✅ **ESP32 device** with WiFi + MQTT + TLS client auth
* ✅ **TLS mutual authentication** via custom certificates
* ✅ **HMAC-SHA256** signature generation on device
* ✅ **JSON payload** with: `steps`, `timestamp`, `nonce`, `signature`
* ✅ **Rust backend**:

  * MQTT TLS client
  * HMAC & timestamp validation
  * PDA-based step tracking and token minting
* ✅ **Solana Anchor program** with `log_step` instruction
* ✅ **Solana Devnet** deployment and testing

---

## 🔐 Security Architecture

* HMAC-SHA256 signed payloads (shared secret)
* Timestamp validation to prevent replay attacks (±30s)
* TLS mutual authentication (ESP32 ↔ EMQX ↔ backend)
* EMQX Broker enforces certificate-based access and ACL rules
* Backend runs on a hardened Google Cloud VPS with TLS
* PDA ensures unique, tamper-proof on-chain logs per `(user, day)`

---

## 📁 Project Structure

```
GreenGait/
├── backend/              # Rust backend (MQTT client, validation, blockchain interaction)
├── solana_program/       # Anchor smart contract + TypeScript tests
├── solana/               # CLI scripts, account utilities, program deploy
├── frontend/             # (WIP) UI for displaying step history and rewards
├── firmware/             # ESP32 Arduino code (WiFi, MQTT, HMAC)
├── docs/                 # PPT Presentation + Logo
└── README.md             # You're here!
```

---

## 🔁 Example Flow

1. Press the button → ESP32 sends a signed JSON payload
2. EMQX broker securely forwards it to the backend
3. Backend verifies the HMAC + timestamp → logs it on-chain
4. If steps are divisible by 3, a token is minted
5. Frontend displays user stats (WIP)

---

## 🛠 How to Run Locally

### 1. Flash the ESP32

Upload `ESP32.ino` from `firmware/` using Arduino IDE.
Make sure `certificates.h` contains:

* `ca.crt`
* `client.crt`
* `client.key`

### 2. Start the Rust Backend

```bash
cd backend
cargo run
```

Make sure these files exist in `certs/`:

```
ca.crt
client.crt
client.key
stepmint-validator.json (Solana keypair)
```

### 3. Run the Solana Tests

```bash
cd solana_program
anchor test
```

---

## 🌟 What's Next?

* [ ] 🧠 PDA optimization + on-chain compression
* [ ] 💎 NFT/token design for major milestones
* [ ] 🎨 Dashboard UI with wallet connection and real-time stats
* [ ] 🔄 ESP32 OTA firmware delivery
* [ ] 🛡️ Replay prevention & abuse detection

---

## 🚀 Live Preview

A hosted UI is coming soon — where users can track their steps, tokens, and compete on the leaderboard 🌍

---

## 👤 Author

**Robert Panța**
MSc Student in Cybersecurity @ Technical University of Cluj-Napoca

* 📧 [LinkedIn](https://www.linkedin.com/in/robert-panta/)
* 🌐 [GitHub](https://github.com/RobCyberLab)

---

> 🍃 *GreenGait – where every step counts… on-chain, securely, and sustainably.*
