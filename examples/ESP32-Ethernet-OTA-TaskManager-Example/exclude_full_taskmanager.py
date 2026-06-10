# ESP32-TaskManager ships both TaskManager.cpp (optimized, default) and
# TaskManager_Full.cpp. PlatformIO compiles every .cpp in a library's src/,
# so both translation units define class TaskManager -> multiple-definition
# link errors. The optimized variant is the default (USE_FULL_TASKMANAGER 0),
# so neutralize the Full variant's body in the resolved dependency copy.
import os
Import("env")

libdeps = env.subst("$PROJECT_LIBDEPS_DIR")
envname = env.subst("$PIOENV")
full = os.path.join(libdeps, envname, "ESP32-TaskManager", "src", "TaskManager_Full.cpp")
if os.path.isfile(full):
    with open(full, "r") as f:
        content = f.read()
    if not content.lstrip().startswith("#if USE_FULL_TASKMANAGER"):
        with open(full, "w") as f:
            f.write("// Neutralized by example: optimized TaskManager.cpp is the default.\n")
        print("exclude_full_taskmanager: emptied %s" % full)
