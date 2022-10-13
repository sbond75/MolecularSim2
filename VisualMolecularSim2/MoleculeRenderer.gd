extends MultiMeshInstance


# Declare member variables here. Examples:
# var a = 2
# var b = "text"


# Called when the node enters the scene tree for the first time.
func _ready():
	for i in range(self.multimesh.instance_count):
		var position = Transform()
		var range_=10
		position = position.translated(Vector3(randf() * range_ - range_/2, randf() * range_ - range_/4, randf() * range_/2 - range_/4))
		self.multimesh.set_instance_transform(i, position)


# Called every frame. 'delta' is the elapsed time since the previous frame.
#func _process(delta):
#	pass
