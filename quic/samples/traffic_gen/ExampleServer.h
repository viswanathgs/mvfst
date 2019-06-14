/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 */

#pragma once

#include <glog/logging.h>

#include <quic/common/test/TestUtils.h>
#include <quic/server/QuicServer.h>
#include <quic/server/QuicServerTransport.h>
#include <quic/server/QuicSharedUDPSocketFactory.h>

#include <quic/samples/traffic_gen/ExampleHandler.h>

namespace quic {
namespace traffic_gen {

class ExampleServerTransportFactory : public quic::QuicServerTransportFactory {
 public:
  ~ExampleServerTransportFactory() override {
    while (!exampleHandlers_.empty()) {
      auto& handler = exampleHandlers_.back();
      handler->getEventBase()->runImmediatelyOrRunInEventBaseThreadAndWait(
          [this] {
            // The evb should be performing a sequential consistency atomic
            // operation already, so we can bank on that to make sure the writes
            // propagate to all threads.
            exampleHandlers_.pop_back();
          });
    }
  }

  ExampleServerTransportFactory() {}

  quic::QuicServerTransport::Ptr make(
      folly::EventBase* evb, std::unique_ptr<folly::AsyncUDPSocket> sock,
      const folly::SocketAddress&,
      std::shared_ptr<const fizz::server::FizzServerContext>
          ctx) noexcept override {
    CHECK_EQ(evb, sock->getEventBase());
    auto exampleHandler = std::make_unique<ExampleHandler>(evb);
    auto transport = quic::QuicServerTransport::make(evb, std::move(sock),
                                                     *exampleHandler, ctx);
    exampleHandler->setQuicSocket(transport);
    exampleHandlers_.push_back(std::move(exampleHandler));
    return transport;
  }

  std::vector<std::unique_ptr<ExampleHandler>> exampleHandlers_;

 private:
};

class ExampleServer {
 public:
  explicit ExampleServer(const std::string& host = "::1", uint16_t port = 6666)
      : host_(host), port_(port), server_(QuicServer::createQuicServer()) {
    server_->setQuicServerTransportFactory(
        std::make_unique<ExampleServerTransportFactory>());
    server_->setFizzContext(quic::test::createServerCtx());
    TransportSettings settings;
    server_->setTransportSettings(settings);
  }

  void start() {
    // Create a SocketAddress and the default or passed in host.
    folly::SocketAddress addr1(host_.c_str(), port_);
    addr1.setFromHostPort(host_, port_);
    server_->start(addr1, 0);
    LOG(INFO) << "Example server started at: " << addr1.describe();
    eventbase_.loopForever();
  }

 private:
  std::string host_;
  uint16_t port_;
  folly::EventBase eventbase_;
  std::shared_ptr<quic::QuicServer> server_;
};

}  // namespace traffic_gen
}  // namespace quic
