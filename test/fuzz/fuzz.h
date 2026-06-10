#pragma once

#include <cstddef>
#include <cstdint>
#include <chrono>

#include <Dispatcher.h>
#include <Mesh.h>
#include <helpers/BaseChatMesh.h>

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
  void setCurrentTime(uint32_t time) override { }
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
class FuzzerMesh : public BaseChatMesh {
public:
  FuzzerMesh(mesh::Radio& radio,
             mesh::MillisecondClock& ms,
             mesh::RNG& rng,
             mesh::RTCClock& rtc,
             mesh::PacketManager& mgr,
             mesh::MeshTables& tables)
    : BaseChatMesh(radio, ms, rng, rtc, mgr, tables) {}

  bool allowPacketForward(const mesh::Packet* packet) override {
    return true;
  }

  void onDiscoveredContact(ContactInfo& contact, bool is_new, uint8_t path_len, const uint8_t* path) override { }

  ContactInfo* processAck(const uint8_t *data) override {
    return nullptr;
  }

  void onContactPathUpdated(const ContactInfo& contact) override { }

  void onMessageRecv(const ContactInfo& contact, mesh::Packet* pkt, uint32_t sender_timestamp, const char *text) override { }

  void onCommandDataRecv(const ContactInfo& contact, mesh::Packet* pkt, uint32_t sender_timestamp, const char *text) override { }

  void onSignedMessageRecv(const ContactInfo& contact, mesh::Packet* pkt, uint32_t sender_timestamp, const uint8_t *sender_prefix, const char *text) override { }

  uint32_t calcFloodTimeoutMillisFor(uint32_t pkt_airtime_millis) const override {
    return pkt_airtime_millis + 1;
  }

  uint32_t calcDirectTimeoutMillisFor(uint32_t pkt_airtime_millis, uint8_t path_len) const override {
    return pkt_airtime_millis + path_len + 1;
  }

  void onSendTimeout() override { }

  void onChannelMessageRecv(const mesh::GroupChannel& channel, mesh::Packet* pkt, uint32_t timestamp, const char *text) override { }

  uint8_t onContactRequest(const ContactInfo& contact, uint32_t sender_timestamp, const uint8_t* data, uint8_t len, uint8_t* reply) override {
    return 0;
  }

  void onContactResponse(const ContactInfo& contact, const uint8_t* data, uint8_t len) override { }

  bool shouldOverwriteWhenFull() const override {
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
