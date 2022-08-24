@tool
extends BehaviourTreeCustomActionNode
class_name SimpleRotationBTreeNode

var cur_node:Node2D;

func _on_btnode_initialize():
	var btree = get_behaviour_tree() as BehaviourTree
	cur_node = btree.get_blackboard("bt_target_node") as Node2D

func _on_btnode_enter():
	pass

func _on_btnode_execute():
	var delta = deg2rad(170 * cur_node.get_process_delta_time())
	var rotation = wrapf(cur_node.global_rotation + delta, 0, 2 * PI)
	cur_node.global_rotation = rotation
	if (rotation >= (2 * PI - 0.05)):
		cur_node.global_rotation = 0
		return BehaviourTree.BEHAVIOUR_TREE_NODE_SUCCESS
	return BehaviourTree.BEHAVIOUR_TREE_NODE_RUNNING

func _on_btnode_exit():
	pass
