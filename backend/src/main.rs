// src/main.rs

// mod config;
// mod security;
// mod mqtt;
// mod blockchain;

// #[tokio::main]
// async fn main() {
//     println!("[SYSTEM] GreenGait Backend Validator Starting...");
//     mqtt::start_mqtt().await;
// }




mod config;
mod security;
mod mqtt;
mod blockchain;
mod state;

use axum::{Router, Json, routing::get};
use tower_http::services::ServeDir;
use std::{net::SocketAddr, sync::{Arc, Mutex}};
use crate::state::{STATUS, StepInfo};

/// Endpoint REST pentru UI – returnează starea curentă (pași și tokeni)
async fn get_status() -> Json<StepInfo> {
    Json(STATUS.lock().unwrap().clone())
}

#[tokio::main]
async fn main() {
    println!("[SYSTEM] GreenGait Backend Validator Starting...");

    // 🧠 Rulează Web UI + REST API pe un thread separat
    tokio::spawn(async {
        let app = Router::new()
            .route("/status", get(get_status)) // Endpoint API
            .nest_service("/", axum::routing::get_service(ServeDir::new("../frontend"))); // UI static

        let addr = SocketAddr::from(([0, 0, 0, 0], 3000));
        println!("🌍 Web dashboard available at: http://localhost:3000");
        let listener = tokio::net::TcpListener::bind(addr).await.unwrap();
        axum::serve(listener, app).await.unwrap();
    });

    // 🔗 Ascultă MQTT pentru ESP32 în threadul principal
    mqtt::start_mqtt().await;
}
