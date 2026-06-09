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
  static StaticPoolPacketManager mgr(1);
  static SimpleMeshTables tables;
  static FuzzerMesh mesh(radio, ms, rng, rtc, mgr, tables);

  auto pkt = mesh.obtainNewPacket();
  if (!pkt) {
    __builtin_trap();
  }
  auto result = mesh.tryParsePacket(pkt, data, size);
  if (result) {
    mesh.testOnRecvPacket(pkt);
  }
  mesh.releasePacket(pkt);

  return 0;
}
