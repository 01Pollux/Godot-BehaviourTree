extends Node

@onready var exec_tree = preload("res://new_v_behaviour_tree_resource.vbtree")
@onready var tree_holder = $VisualBTree as BehaviourTreeRemoteTreeHolder

func _ready():
	tree_holder.vbehaviour_tree = exec_tree

func _process(_delta):
	exec_tree.execute_tree()
