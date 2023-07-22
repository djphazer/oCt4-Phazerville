Import("env")

import subprocess

version = subprocess.check_output("sh ./scripts/build_version.sh", shell=True).decode().strip()
git_rev = subprocess.check_output("git rev-parse --short HEAD", shell=True).decode().strip()

env.Append(BUILD_FLAGS=[ f'-DOCT4_BUILD_VERSION=\\"{version}\\"',
                         f'-DOCT4_BUILD_TAG=\\"{git_rev}\\"' ])

env.Replace(PROGNAME=f"oCt4-phazerville-{version}-{git_rev}")
