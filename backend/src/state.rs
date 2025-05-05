use std::sync::{Arc, Mutex};
use once_cell::sync::Lazy;
use serde::Serialize;

#[derive(Serialize, Clone)]
pub struct StepInfo {
    pub steps: u64,
    pub tokens: u64,
}

pub static STATUS: Lazy<Arc<Mutex<StepInfo>>> = Lazy::new(|| {
    Arc::new(Mutex::new(StepInfo { steps: 0, tokens: 0 }))
});
