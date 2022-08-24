@tool
extends BehaviourTreeCustomActionNode
class_name SimpleTranslationBTreeNode

@export var translate_left:bool = false

var cur_node:Node2D
var start_position

func _on_btnode_initialize():
	var btree = get_behaviour_tree() as BehaviourTree
	cur_node = btree.get_blackboard("bt_target_node") as Node2D

func _on_btnode_enter():
	start_position = cur_node.global_position
	pass

func _on_btnode_execute():
	cur_node.translate(Vector2(-5 if translate_left else 5, 0))
	if (cur_node.global_position.distance_to(start_position) >= 100):
		return BehaviourTree.BEHAVIOUR_TREE_NODE_SUCCESS
	return BehaviourTree.BEHAVIOUR_TREE_NODE_RUNNING

func _on_btnode_exit():
	pass
