# config.py

def can_build(env, platform):
    return True


def get_doc_classes():
    return [
        "BehaviourTree",
        "BehaviourTreeCustomActionNode"
    ]

def get_doc_path():
    return "doc_classes"

def configure(env):
    pass
