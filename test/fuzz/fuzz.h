#pragma once

#include <cstddef>
#include <cstdint>
#include <chrono>

#include <Dispatcher.h>
#include <Mesh.h>

/**
 * \brief  Implementation of the mesh::Radio interface, for use in fuzz testing.
*/
class FuzzerRadio : public mesh::Radio {
public:
  int recvRaw(uint8_t* bytes, int sz) override {
    return 0;
  }

  uint32_t getEstAirtimeFor(int len_bytes) override {
    return 14 * len_bytes;
  }

  float packetScore(float snr, int packet_len) override {
      return 0;
  }

  bool startSendRaw(const uint8_t* bytes, int len) override {
      return true;
  }

  bool isSendComplete() override {
      return true;
  }

  void onSendFinished() override { }

  bool isInRecvMode() const override {
      return true;
  }
};

/**
 * \brief  Implementation of the mesh::MillisecondClock interface, for use in fuzz testing.
 */
class FuzzerMillis : public mesh::MillisecondClock {
public:
  unsigned long getMillis() override {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    return millis;
  }
};

/**
 * \brief  Implementation of the mesh::RTCClock interface, for use in fuzz testing.
 */
class FuzzerRTC : public mesh::RTCClock {
public:
  FuzzerRTC() { }
  uint32_t getCurrentTime() override {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
    return seconds;
  }
  void setCurrentTime(uint32_t time) override {  }
};

/**
 * \brief  Implementation of the mesh::RNG interface, for use in fuzz testing. Returns a constant value.
 */
class FuzzerRNG : public mesh::RNG {
public:
  void random(uint8_t* dest, size_t sz) override {
    memset(dest, 42, sz);
  }
};

/**
 * \brief  Implementation of the mesh::Mesh class, for use in fuzz testing.
 *
 * Exposes the protected onRecvPacket() method as testOnRecvPacket() for fuzz testing.
 */
class FuzzerMesh : public mesh::Mesh {
public:
  FuzzerMesh(mesh::Radio& radio,
             mesh::MillisecondClock& ms,
             mesh::RNG& rng,
             mesh::RTCClock& rtc,
             mesh::PacketManager& mgr,
             mesh::MeshTables& tables)
    : mesh::Mesh(radio, ms, rng, rtc, mgr, tables) {}

  bool allowPacketForward(const mesh::Packet* packet) override {
    return true;
  }

  int searchChannelsByHash(const uint8_t* hash, mesh::GroupChannel dest[], int max_matches) override {
    int n = 0;
    if (hash[0] == 0x11) {
      // known "Public" channel
      dest[n++] = { .hash = {0x11}, .secret = {0x8b, 0x33, 0x87, 0xe9, 0xc5, 0xcd, 0xea, 0x6a, 0xc9, 0xe5, 0xed, 0xba, 0xa1, 0x15, 0xcd, 0x72} };
    } else if (hash[0] == 0xd9) {
      // known "#test" channel
      dest[n++] = { .hash = {0xd9}, .secret = {0x9c, 0xd8, 0xfc, 0xf2, 0x2a, 0x47, 0x33, 0x3b, 0x59, 0x1d, 0x96, 0xa2, 0xb8, 0x48, 0xb7, 0x3f} };
    }
    return n;
  }

  // onRecvPacket is protected; expose it for fuzz testing.
  mesh::DispatcherAction testOnRecvPacket(mesh::Packet* pkt) {
    return onRecvPacket(pkt);
  }
};
