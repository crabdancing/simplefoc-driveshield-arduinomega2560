import os
Import("env")

# include toolchain paths
env.Replace(COMPILATIONDB_INCLUDE_TOOLCHAIN=True)

# override compilation DB path
env.Replace(COMPILATIONDB_PATH="compile_commands.json")

# env.Append(
#     BUILD_FLAGS=[
#         "-mfloat-abi=soft",
#         "-D__FPU_PRESENT=0"
#     ]
# )
