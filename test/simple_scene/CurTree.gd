extends Sprite2D

@onready var behaviour_tree:BehaviourTree = $BehaviourTree.MainTree


func _ready():
	behaviour_tree.set_blackboard("bt_target_node", self)
	behaviour_tree.initialize_tree()


func _process(delta):
	behaviour_tree.execute_tree()
	pass
	
signal _on_bt_callback(val0, val1);

func _on_node_2d_on_bt_callback(val0, val1):
	print("called with ", val0, " ", val1)
