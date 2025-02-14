/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 */

#include <quic/congestion_control/CongestionControllerFactory.h>

#include <quic/congestion_control/Copa.h>
#include <quic/congestion_control/NewReno.h>
#include <quic/congestion_control/QuicCubic.h>
#include <quic/congestion_control/rl/RLCongestionController.h>

#include <memory>

namespace quic {
std::unique_ptr<CongestionController>
DefaultCongestionControllerFactory::makeCongestionController(
    QuicConnectionStateBase& conn,
    CongestionControlType type) {
  std::unique_ptr<CongestionController> congestionController;
  switch (type) {
    case CongestionControlType::NewReno:
      congestionController = std::make_unique<NewReno>(conn);
      break;
    case CongestionControlType::Cubic:
      congestionController = std::make_unique<Cubic>(conn);
      break;
    case CongestionControlType::Copa:
      congestionController = std::make_unique<Copa>(conn);
      break;
    case CongestionControlType::BBR:
      throw std::runtime_error("Unsupported Congestion Control Algorithm");
    case CongestionControlType::RL:
      congestionController = std::make_unique<RLCongestionController>(conn);
      break;
    case CongestionControlType::None:
      break;
  }
  return congestionController;
}
} // namespace quic
