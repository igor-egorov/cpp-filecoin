/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CPP_FILECOIN_REWARD_ACTOR_HPP
#define CPP_FILECOIN_REWARD_ACTOR_HPP

#include <boost/optional.hpp>
#include "codec/cbor/streams_annotation.hpp"
#include "common/enum.hpp"
#include "power/power_table.hpp"
#include "primitives/address/address.hpp"
#include "primitives/cid/cid.hpp"
#include "primitives/types.hpp"
#include "storage/ipfs/datastore.hpp"
#include "vm/actor/actor.hpp"
#include "vm/actor/actor_method.hpp"

namespace fc::vm::actor::builtin::reward {

  using power::Power;
  using primitives::TokenAmount;

  enum class VestingFunction : uint64_t {
    NONE = 0,
    LINEAR,
  };

  struct Reward {
    VestingFunction vesting_function;
    primitives::ChainEpoch start_epoch;
    primitives::ChainEpoch end_epoch;
    TokenAmount value;
    TokenAmount amount_withdrawn;

    primitives::BigInt amountVested(
        const primitives::ChainEpoch &current_epoch);
  };

  CBOR_TUPLE(
      Reward, vesting_function, start_epoch, end_epoch, value, amount_withdrawn)

  struct State {
    TokenAmount reward_total;
    CID reward_map;

    outcome::result<void> addReward(
        const std::shared_ptr<storage::ipfs::IpfsDatastore> &store,
        const Address &owner,
        const Reward &reward);

    outcome::result<TokenAmount> withdrawReward(
        const std::shared_ptr<storage::ipfs::IpfsDatastore> &store,
        const Address &owner,
        const primitives::ChainEpoch &current_epoch);
  };

  CBOR_TUPLE(State, reward_map, reward_total)

  // Actor related stuff

  /**
   * The network works purely in the indivisible token amounts. This constant
   * converts to a fixed decimal with more human-friendly scale.
   */
  static const BigInt kTokenPrecision{1e18};

  /**
   * Target reward released to each block winner
   */
  static const BigInt kBlockRewardTarget{1e20};

  static constexpr auto kRewardVestingFunction{VestingFunction::NONE};
  static const primitives::EpochDuration kRewardVestingPeriod{0};

  ACTOR_METHOD_DECL(1, Construct){};

  ACTOR_METHOD_DECL(2, AwardBlockReward) {
    struct Params {
      Address miner;
      TokenAmount penalty;
      TokenAmount gas_reward;
      Power nominal_power;
    };
  };

  ACTOR_METHOD_DECL(3, WithdrawReward){};

  CBOR_TUPLE(
      AwardBlockReward::Params, miner, penalty, gas_reward, nominal_power)

  extern const ActorExports exports;

  TokenAmount computeBlockReward(const State &state,
                                 const TokenAmount &balance);
}  // namespace fc::vm::actor::builtin::reward

#endif  // CPP_FILECOIN_REWARD_ACTOR_HPP
