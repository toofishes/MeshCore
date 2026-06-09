# This file is used by the fuzzing build configuration to set up clang for coverage builds.
# It is referenced via `extra_scripts` in the platformio configuration, and must live next to platformio.ini.

Import("env")
Import("projenv")

def configure_clang(e):
    e['CC'] = 'clang'
    e['CXX'] = 'clang++'
    e['CCDEPFLAGS'] = '-MMD -MF ${TARGET}.d'
    e['LINK'] = 'clang++'
    e["NINJA_DEPFILE_PARSE_FORMAT"] = 'clang'
    e.Append(
        LINKFLAGS=["-fprofile-instr-generate", "-fcoverage-mapping"],
    )

for e in [env, projenv]:
    configure_clang(e)

for lb in env.GetLibBuilders():
    configure_clang(lb.env)
