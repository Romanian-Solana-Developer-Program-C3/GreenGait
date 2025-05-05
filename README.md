# 🍃 GreenGait – Step Into Web3 Rewards 🍃

**GreenGait** is a Web3 rewards platform that transforms physical activity into real digital value using the Solana blockchain. With a secure, Wi-Fi-enabled wearable device (ESP32), every step you take is cryptographically signed and submitted to the blockchain – all in real time.
🏆️️ + ✅ → 💰 on-chain.

---

## 🌐 Architecture Overview

🔹 **ESP32-WROOM-32D**
A microcontroller that simulates steps via a button. Sends step data via MQTT over TLS using mutual certificate authentication.

🔐 **TLS Mutual Authentication**
Secure communication using custom client certificates and a trusted CA, protecting against unauthorized devices.

🧠 **Rust Backend Validator**
Receives messages via MQTT, validates their authenticity (HMAC + timestamp), then logs valid steps on-chain through a Solana program.

⛓️ **Solana Anchor Program**
Smart contract deployed on Devnet, using Program Derived Addresses (PDAs) to store step data per user per day. Also supports token minting for every 3 steps.

📡 **EMQX Broker (on Google Cloud VPS)**
Handles secure MQTT message routing, enforcing TLS and ACL-based authentication.

🖥️ **Frontend Interface** *(WIP)*
Displays step history and reward data tied to each wallet.

---

## ✅ Features Implemented

* ✅ **ESP32 device** with WiFi + MQTT + TLS client auth
* ✅ **TLS Mutual Authentication** (EMQX broker with CA and client certs)
* ✅ **HMAC-SHA256** signature from the device
* ✅ **JSON payload**: steps, timestamp, nonce, and signature
* ✅ **Rust backend**:

  * MQTT client with client cert
  * HMAC + timestamp verification
  * PDA derivation per `(user, day)`
  * On-chain logging + token minting (3 steps = 1 token)
* ✅ **Anchor program** with `log_step` instruction
* ✅ \*\*Solana Devnet deployment\`

---

## 🔐 Security Architecture

* HMAC-SHA256 signed payloads (shared secret)
* Timestamp validation (±30s) to prevent replay attacks
* TLS mutual authentication (ESP32 ↔ EMQX ↔ backend)
* Backend uses a hardened VPS (GCP) + cert-based MQTT
* PDA ensures unique per-user per-day data segregation

---

## 📁 Project Structure

```
greengait_project/
├── backend_rust/         # Rust backend (MQTT client, validation, blockchain interaction)
├── greengait_program/    # Anchor smart contract + TypeScript tests
├── greengait_solana/     # CLI scripts, account utilities, program deploy
├── greengait_frontend/   # (WIP) UI for displaying step history and rewards
├── certs/                # ca.crt, client.crt, client.key, validator keypair
├── firmware/             # ESP32 Arduino code (WiFi, MQTT, HMAC)
└── README.md             # You're here!
```

---

## 🦪 Example Flow

1. Press the button → ESP32 sends a JSON payload with steps, timestamp, HMAC
2. MQTT broker forwards it securely to the backend
3. Backend verifies payload → signs & sends a transaction to Solana
4. Anchor program logs step data (per day) & mints token if needed
5. User can view total steps and rewards in the frontend (coming soon)

---

## 🛠 How to Run Locally

### 1. Flash the ESP32

Upload `ESP32.ino` from the `firmware/` folder using Arduino IDE.
Ensure `certificates.h` contains your TLS client cert/key and CA.

### 2. Start the Rust Backend

```bash
cd backend_rust
cargo run
```

Ensure the following files exist in `certs/`:

```
ca.crt
client.crt
client.key
stepmint-validator.json (Solana keypair)
```

### 3. Run Tests

```bash
cd solana_program
anchor test
```

---

## 🌟 What's Next?

* [ ] 🧠 PDA optimization + on-chain compression
* [ ] 💎 NFT/token design for milestones
* [ ] 🎨 UI dashboard for wallet-based stats
* [ ] 🔄 ESP32 OTA firmware updates
* [ ] 🛡️ Anti-spoofing + abuse protection

---

## 🚀 Live Preview (Soon)

Coming soon: Hosted UI for interacting with your GreenGait account, token rewards, and leaderboard integration.

---

## 👤 Author

**Robert Panța**
MSc Student in Cybersecurity @ Technical University of Cluj-Napoca

* 📧 [LinkedIn](https://www.linkedin.com/in/robert-panta/)
* 🌐 [GitHub](https://github.com/RobCyberLab)

---

> 🍃 *GreenGait – where every step counts... on-chain.*
