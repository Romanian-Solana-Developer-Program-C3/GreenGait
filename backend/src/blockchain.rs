use anchor_client::solana_sdk::{
    pubkey::Pubkey,
    signature::{read_keypair_file, Signer},
    system_program,
    sysvar,
};
use anchor_client::Client;
use anchor_client::Cluster;
use greengait_program::instruction::LogStep;
use greengait_program::accounts::LogStep as LogStepAccounts;
// use anchor_client::solana_sdk::pubkey::Pubkey;
use spl_associated_token_account::get_associated_token_address;
use spl_token;


use std::rc::Rc;
use std::str::FromStr;

// pub async fn log_step_on_chain(
//     user_pubkey: &str,
//     steps: u64,
//     day: i64,
// ) -> anyhow::Result<String> {
pub async fn log_step_on_chain(
    user_pubkey: &str,
    steps: u64,
    day: i64,
    mint_address: &str,
) -> anyhow::Result<String> {
    
    let payer = Rc::new(read_keypair_file("certs/greengait-validator.json")
        .expect("❌ Cannot read validator keypair"));
    let user = Pubkey::from_str(user_pubkey)?;
    let mint = Pubkey::from_str(mint_address)?;
    let user_ata = get_associated_token_address(&user, &mint);

    let program_id = Pubkey::from_str("DYf1G6UEyAebNDfgkQodKUvPBAZJXMCkBtWgRMpS8SaE")?;
    let (pda, _bump) = Pubkey::find_program_address( &[b"step_data", user.as_ref(), &day.to_le_bytes()], &program_id);


    // ✅ DEBUG PRINTS
    println!("[DEBUG] Payer pubkey: {}", payer.pubkey());
    println!("[DEBUG] User pubkey: {}", user);
    println!("[DEBUG] Program ID: {}", program_id);
    println!("[DEBUG] Mint address: {}", mint);
    println!("[DEBUG] User ATA: {}", user_ata);
    println!("[DEBUG] Derived PDA: {}", pda);
    let client = Client::new(Cluster::Devnet, payer.clone());
    let program = client.program(program_id)?; // ✅ good

    let tx = program
        .request()
        // .accounts(LogStepAccounts {
        //     user,
        //     step_data: pda,
        //     payer: payer.pubkey(), // ✅ Add this field
        //     system_program: anchor_client::solana_sdk::system_program::ID,
        // })
        .accounts(LogStepAccounts {
            user,
            step_data: pda,
            payer: payer.pubkey(),
            mint,
            user_ata,
            system_program: system_program::ID,
            token_program: spl_token::ID,
            // associated_token_program: spl_associated_token_account::ID,
            rent: sysvar::rent::ID,
        })
        
        
        .args(LogStep {
            steps,
            day,
        })
        .send()
        .await?; 

    println!("[CHAIN] ✅ Anchor program called. Tx: {}", tx);
    Ok(tx.to_string())
}  