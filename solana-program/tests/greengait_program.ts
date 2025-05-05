// import * as anchor from "@coral-xyz/anchor";
// import { Program } from "@coral-xyz/anchor";
// import { GreengaitProgram } from "../target/types/greengait_program";

// describe("greengait_program", () => {
//   // Configure the client to use the local cluster.
//   anchor.setProvider(anchor.AnchorProvider.env());

//   const program = anchor.workspace.GreengaitProgram as Program<GreengaitProgram>;

//   it("Is initialized!", async () => {
//     // Add your test here.
//     // const tx = await program.methods.initialize().rpc();
//     // console.log("Your transaction signature", tx);
//   });
// });

import * as anchor from "@coral-xyz/anchor";
import { Program } from "@coral-xyz/anchor";
import { GreengaitProgram } from "../target/types/greengait_program";
// @ts-ignore
import {
  createMint,
  getAssociatedTokenAddress,
  createAssociatedTokenAccount,
  getAccount,
} from "@solana/spl-token";
import { assert } from "chai";

describe("greengait_program", () => {
  const provider = anchor.AnchorProvider.env();
  anchor.setProvider(provider);
  const program = anchor.workspace.GreengaitProgram as Program<GreengaitProgram>;
  const connection = provider.connection;
  const signer = (provider.wallet as anchor.Wallet).payer;

  const mintAndPrepare = async () => {
    const mint = await createMint(connection, signer, signer.publicKey, null, 9);
    const userAta = await getAssociatedTokenAddress(mint, signer.publicKey);
    await createAssociatedTokenAccount(connection, signer, mint, signer.publicKey);
    return { mint, userAta };
  };

  const deriveStepDataPda = (userPubkey: anchor.web3.PublicKey, day: number) => {
    return anchor.web3.PublicKey.findProgramAddressSync(
      [
        Buffer.from("step_data"),
        userPubkey.toBuffer(),
        new anchor.BN(day).toArrayLike(Buffer, "le", 8),
      ],
      program.programId
    );
  };

  it("✅ Minting tokens for 6 steps", async () => {
    const { mint, userAta } = await mintAndPrepare();
    const generateUniqueDay = () => {
      const now = Date.now(); // timestamp în milisecunde
      return Math.floor(now / 1000); // îl facem UNIX timestamp în secunde
    };
    
    const day = generateUniqueDay();
    const [stepDataPda] = deriveStepDataPda(signer.publicKey, day);

    await program.methods
      .logStep(new anchor.BN(6), new anchor.BN(day))
      .accounts({
        user: signer.publicKey,
        stepData: stepDataPda,
        payer: signer.publicKey,
        mint,
        userAta,
        systemProgram: anchor.web3.SystemProgram.programId,
        tokenProgram: anchor.utils.token.TOKEN_PROGRAM_ID,
        rent: anchor.web3.SYSVAR_RENT_PUBKEY,
      } as any)
      .signers([])
      .rpc();

    const stepData = await program.account.stepData.fetch(stepDataPda);
    assert.equal(stepData.steps.toNumber(), 6);
    assert.equal(stepData.lastMinted.toNumber(), 6);

    const tokenAccount = await getAccount(connection, userAta);
    assert.equal(Number(tokenAccount.amount), 2_000_000_000);
  });

  it("❌ Not minting tokens for 2 steps", async () => {
    const { mint, userAta } = await mintAndPrepare();
    const generateUniqueDay = () => {
      const now = Date.now(); // timestamp în milisecunde
      return Math.floor(now / 1000); // îl facem UNIX timestamp în secunde
    };
    
    const day = generateUniqueDay();
    const [stepDataPda] = deriveStepDataPda(signer.publicKey, day);

    await program.methods
      .logStep(new anchor.BN(2), new anchor.BN(day))
      .accounts({
        user: signer.publicKey,
        stepData: stepDataPda,
        payer: signer.publicKey,
        mint,
        userAta,
        systemProgram: anchor.web3.SystemProgram.programId,
        tokenProgram: anchor.utils.token.TOKEN_PROGRAM_ID,
        rent: anchor.web3.SYSVAR_RENT_PUBKEY,
      } as any)
      .signers([])
      .rpc();

    const stepData = await program.account.stepData.fetch(stepDataPda);
    assert.equal(stepData.steps.toNumber(), 2);
    assert.equal(stepData.lastMinted.toNumber(), 0);

    const tokenAccount = await getAccount(connection, userAta);
    assert.equal(Number(tokenAccount.amount), 0);
  });
});


