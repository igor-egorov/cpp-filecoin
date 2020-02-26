/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CPP_FILECOIN_VM_ACTOR_BUILTIN_MULTISIG_ACTOR_HPP
#define CPP_FILECOIN_VM_ACTOR_BUILTIN_MULTISIG_ACTOR_HPP

#include "codec/cbor/streams_annotation.hpp"
#include "primitives/address/address.hpp"
#include "primitives/address/address_codec.hpp"
#include "primitives/big_int.hpp"
#include "primitives/chain_epoch/chain_epoch.hpp"
#include "primitives/cid/cid.hpp"
#include "vm/actor/actor.hpp"
#include "vm/actor/actor_method.hpp"
#include "vm/runtime/runtime.hpp"

namespace fc::vm::actor::builtin::multisig {

  using primitives::ChainEpoch;
  using primitives::EpochDuration;
  using primitives::address::Address;
  using runtime::InvocationOutput;
  using runtime::Runtime;
  using TransactionNumber = size_t;

  /**
   * Multisignaure pending transaction
   */
  struct MultiSignatureTransaction {
    /** Transaction id given by Multisignature Actor */
    TransactionNumber transaction_number;
    Address to;
    BigInt value;
    MethodNumber method;
    MethodParams params;

    /**
     * @brief List of addresses that approved transaction
     * This address at index 0 is the transaction proposer, order of this slice
     * must be preserved
     */
    std::vector<Address> approved;

    bool operator==(const MultiSignatureTransaction &other) const;
  };

  /**
   * State of Multisig Actor instance
   */
  struct MultiSignatureActorState {
    std::vector<Address> signers;
    size_t threshold;
    /** Transaction counter */
    TransactionNumber next_transaction_id;

    // Linear lock
    BigInt initial_balance;
    ChainEpoch start_epoch;
    EpochDuration unlock_duration;

    /** List of pending transactions */
    std::vector<MultiSignatureTransaction> pending_transactions;

    /**
     * Checks if address is signer
     */
    bool isSigner(const Address &address) const;

    /**
     * Checks if address is a creator of transaction
     * @param tx_number - number of transaction to check
     * @param address - address to check
     * @return true if address proposed the transaction
     */
    outcome::result<bool> isTransactionCreator(
        const TransactionNumber &tx_number, const Address &address) const;

    /**
     * Get pending transaction
     */
    outcome::result<MultiSignatureTransaction> getPendingTransaction(
        const TransactionNumber &tx_number) const;

    /**
     * Update pending transaction by transaction_number
     * @param transaction - to update
     * @return nothing or error occurred
     */
    outcome::result<void> updatePendingTransaction(
        const MultiSignatureTransaction &transaction);

    /**
     * Delete pending transaction by tx_number
     * @param tx_number - to delete
     * @return nothing or error occurred
     */
    outcome::result<void> deletePendingTransaction(
        const TransactionNumber &tx_number);

    /**
     * Approve pending transaction by tx_number. Add signer and if threshold is
     * met send pending tx
     * @param actor - caller
     * @param runtime - execution context
     * @param tx_number - tx to approve
     * @return nothing or error occurred
     */
    outcome::result<void> approveTransaction(
        const Actor &actor,
        Runtime &runtime,
        const TransactionNumber &tx_number);

    /**
     * Get amount locked for current epoch
     * @param current_epoch - current block number
     * @return - return amount locked
     */
    BigInt getAmountLocked(const ChainEpoch &current_epoch) const;
  };

  ACTOR_METHOD_DECL(1, Construct) {
    struct Params {
      std::vector<Address> signers;
      size_t threshold;
      EpochDuration unlock_duration;
    };
  };

  ACTOR_METHOD_DECL(2, Propose) {
    struct Params {
      Address to;
      BigInt value;
      MethodNumber method{};
      MethodParams params;
    };
    using Result = TransactionNumber;
  };

  ACTOR_METHOD_DECL(3, Approve) {
    struct Params {
      TransactionNumber transaction_number;
    };
  };

  ACTOR_METHOD_DECL(4, Cancel) {
    struct Params {
      TransactionNumber transaction_number;
    };
  };

  ACTOR_METHOD_DECL(5, AddSigner) {
    struct Params {
      Address signer;
      bool increase_threshold{false};
    };
  };

  ACTOR_METHOD_DECL(6, RemoveSigner) {
    struct Params {
      Address signer;
      bool decrease_threshold{false};
    };
  };

  ACTOR_METHOD_DECL(7, SwapSigner) {
    struct Params {
      Address old_signer;
      Address new_signer;
    };
  };

  ACTOR_METHOD_DECL(7, ChangeThreshold) {
    struct Params {
      size_t new_threshold;
    };
  };

  /** Exported Multisig Actor methods to invoker */
  extern const ActorExports exports;

  CBOR_TUPLE(MultiSignatureTransaction,
             transaction_number,
             to,
             value,
             method,
             params,
             approved)

  CBOR_TUPLE(MultiSignatureActorState,
             signers,
             threshold,
             next_transaction_id,
             initial_balance,
             start_epoch,
             unlock_duration,
             pending_transactions)

  CBOR_TUPLE(Construct::Params, signers, threshold, unlock_duration)

  CBOR_TUPLE(Propose::Params, to, value, method, params)

  CBOR_TUPLE(Approve::Params, transaction_number)

  CBOR_TUPLE(Cancel::Params, transaction_number)

  CBOR_TUPLE(AddSigner::Params, signer, increase_threshold)

  CBOR_TUPLE(RemoveSigner::Params, signer, decrease_threshold)

  CBOR_TUPLE(SwapSigner::Params, old_signer, new_signer)

  CBOR_TUPLE(ChangeThreshold::Params, new_threshold)

}  // namespace fc::vm::actor::builtin::multisig

#endif  // CPP_FILECOIN_VM_ACTOR_BUILTIN_MULTISIG_ACTOR_HPP
