name = 'xstudio'

version = '0.1.0'

authors = [
    'DNEG',
]

description = '''Media review tool'''

with scope('config') as c:
    # Determine location to release: internal (int) vs external (ext)
    # NOTE: Modify this variable to reflect the current package situation
    release_as = 'ext'

    # The 'c' variable here is actually rezconfig.py
    # 'release_packages_path' is a variable defined inside rezconfig.py

    import os
    if release_as == 'int':
        c.release_packages_path = os.environ['SSE_REZ_REPO_RELEASE_INT']
    elif release_as == 'ext':
        c.release_packages_path = os.environ['SSE_REZ_REPO_RELEASE_EXT']

requires = [
    "openssl-1.1.1",
    "nlohmann_json-3.7.3",
    "pybind11-2.6.2",
    "spdlog-1.9.2",
    "fmt-8.0.1",
    "imath-3.1.5",
    "openexr-3.1.5",
    "actor_framework-0.18.4",
    "ocio-2.1.1",  # not exactly the version they asked
    "glew-2.0.0",  # not exactly the version they asked
    "ffmpeg-5.1.0",
    "qt-5.15",
    "libjpeg",
    "freetype",
]

private_build_requires = [
]

variants = [
    ['platform-linux', 'arch-x86_64', 'os-centos-7'],
]

def pre_build_commands():
    command("source /opt/rh/devtoolset-9/enable")

def commands():
    env.REZ_XSTUDIO_ROOT = '{root}'

uuid = 'repository.xstudio'

