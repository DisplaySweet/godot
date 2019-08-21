def can_build(env, platform):
    return platform == "osx" or platform == "x11"

def configure(env):
    pass

def get_doc_classes():
    return [
        "StreamPeerUnixSocket",
    ]

def get_doc_path():
    return "doc_classes"
