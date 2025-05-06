use anchor_lang::prelude::*;
use anchor_spl::token::{mint_to, MintTo, Token};

declare_id!("5LmnBPt81icjN2GE6o2duFEQGQ6J3dptDhxaaVtz5Wj6");

#[program]
pub mod greengait_program {
    use super::*;

    pub fn log_step(ctx: Context<LogStep>, steps: u64, day: i64) -> Result<()> {
        let step_data = &mut ctx.accounts.step_data;

        if step_data.day != day {
            step_data.steps = 0;
            step_data.last_minted = 0;
        }

        step_data.steps += steps;
        step_data.day = day;

        let new_mints = (step_data.steps / 3) - (step_data.last_minted / 3);

        if new_mints > 0 {
            mint_to(
                CpiContext::new(
                    ctx.accounts.token_program.to_account_info(),
                    MintTo {
                        mint: ctx.accounts.mint.to_account_info(),
                        to: ctx.accounts.user_ata.to_account_info(),
                        authority: ctx.accounts.payer.to_account_info(),
                    },
                ),
                new_mints * 1_000_000_000,
            )?;
            step_data.last_minted = step_data.steps;
        }

        Ok(())
    }
}

#[derive(Accounts)]
#[instruction(steps: u64, day: i64)]
pub struct LogStep<'info> {
    /// CHECK: Used only for PDA derivation, this field is safe because it's managed by the Anchor framework
    pub user: AccountInfo<'info>,

    #[account(
        init_if_needed,
        payer = payer,
        seeds = [b"step_data", user.key.as_ref(), &day.to_le_bytes()],
        bump,
        space = 8 + 8 + 8 + 8
    )]
    pub step_data: Account<'info, StepData>,

    #[account(mut, signer)]
    pub payer: Signer<'info>,

    /// CHECK: Verified as a mint account by the token program
    #[account(mut)]
    pub mint: AccountInfo<'info>,

    /// CHECK: Verified as user's associated token account by token program
    #[account(mut)]
    pub user_ata: AccountInfo<'info>,

    pub system_program: Program<'info, System>,
    pub token_program: Program<'info, Token>,
    pub rent: Sysvar<'info, Rent>,
}

#[account]
pub struct StepData {
    pub steps: u64,
    pub day: i64,
    pub last_minted: u64,
}
