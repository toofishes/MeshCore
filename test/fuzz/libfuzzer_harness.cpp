#include <sanitizer/common_interface_defs.h>

#include "fuzz.h"
#include <helpers/SimpleMeshTables.h>
#include <helpers/StaticPoolPacketManager.h>

/**
 * \brief  Fuzzer entry point. Named 'LLVMFuzzerTestOneInput' for libFuzzer to find.
 *
 * This function parses the given input data as a raw packet, and if successful, passes it to the Mesh's onRecvPacket() method.
 */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  static FuzzerRadio radio;
  static FuzzerMillis ms;
  static FuzzerRNG rng;
  static FuzzerRTC rtc;
  static StaticPoolPacketManager mgr(64);
  static SimpleMeshTables tables;
  static FuzzerMesh mesh(radio, ms, rng, rtc, mgr, tables);

  auto pkt = mesh.obtainNewPacket();
  if (!pkt) {
    __builtin_trap();
  }
  auto result = mesh.tryParsePacket(pkt, data, size);
  if (result) {
    uint8_t *path_ptr = &pkt->path[0];
    uint8_t *payload_ptr = &pkt->payload[0];
    __sanitizer_annotate_contiguous_container(path_ptr, path_ptr + MAX_PATH_SIZE,
        path_ptr + MAX_PATH_SIZE, path_ptr + pkt->getPathByteLen());
    __sanitizer_annotate_contiguous_container(payload_ptr, payload_ptr + MAX_PACKET_PAYLOAD,
        payload_ptr + MAX_PACKET_PAYLOAD, payload_ptr + pkt->payload_len);

    mesh.testOnRecvPacket(pkt);

    __sanitizer_annotate_contiguous_container(path_ptr, path_ptr + MAX_PATH_SIZE,
        path_ptr + pkt->getPathByteLen(), path_ptr + MAX_PATH_SIZE);
    __sanitizer_annotate_contiguous_container(payload_ptr, payload_ptr + MAX_PACKET_PAYLOAD,
        payload_ptr + pkt->payload_len, payload_ptr + MAX_PACKET_PAYLOAD);
  }
  mesh.releasePacket(pkt);

  // some code paths queue outbound packets; clean them up before the next fuzz input.
  while (true) {
    auto outbound = mgr.getNextOutbound(UINT32_MAX);
    if (outbound == nullptr) break;
    mesh.releasePacket(outbound);
  }

  return 0;
}
