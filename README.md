# GODOT_BTREE
A statically implemented Behaviour Tree logic in Godot 4.0+

## Results
![image](https://user-images.githubusercontent.com/61026912/183294642-d5ad51b3-4bed-4b14-89ad-3ea86a4a4662.png)


## Installation
1.  Follow the detailed instruction on [how to compile Godot](https://docs.godotengine.org/en/stable/development/compiling/index.html).

2.  Use [the main branch](https://github.com/godotengine/godot) of Godot for Godot 4.0+.

3.  Copy the files (excluding /test/*) into a (/modules/behaviour_tree) in your Godot source folder.

4.  Compile like normally you would do.


## Creating the visual Behaviour Tree
![image](https://user-images.githubusercontent.com/61026912/183295000-5363ebc7-533d-44fa-a142-18ac53e11bdd.png)
1.  Right click on folder / empty section.

2.  Press **New Resources...**.

3.  Search for and add **VBehaviourTreeResource**.


## Editor
![image](https://user-images.githubusercontent.com/61026912/183294882-ad2ac841-76cf-43a5-a782-922f85255451.png)

* Save: Saves the visual behaviour tree into the file.

* Reload: Reload the visual behaviour tree from the file.

* Save as Tree: Saves the visual behaviour tree as runtime behaviour tree.


## Using the runtime Behaviour Tree
* Check the **/test/** in the github repository for the example.

* Normally you would only need to load the runtime tree generated by visual editor and call `get_behaviour_tree()` on it, you could also load the visual behaviour tree but there is not reason to do so.

* Initialize the tree by setting the object it should be targetting `behaviour_tree.set_meta("bt_node_object", self)` (this is optional if you're not using any node that uses **bt_node_object** meta data) and by setting other meta datas.

* Initialize the nodes by calling `behaviour_tree.initialize_tree()`.

* Call `behaviour_tree.execute_tree()` in whatever logic / event you want.


## Adding custom nodes
* Check the **/test/** in the github repository for the example.

* Create a simple json file and reference it from the target visual behaviour tree.

* Each section represents a class name that extends to either `BehaviourTreeCustomActionNode`, `BehaviourTreeCustomCompositeNode` or `BehaviourTreeCustomDecoratorNode`.

* For each section, it contains **name**, **category** and **description**.
