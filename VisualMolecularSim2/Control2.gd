extends Control


# Declare member variables here. Examples:
# var a = 2
# var b = "text"

onready var renderController = $"../RenderController"

# Called when the node enters the scene tree for the first time.
func _ready():
	pass # Replace with function body.


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	$Label.text = "%10.3f ms" % renderController._getLastSimTime() # https://docs.godotengine.org/en/stable/tutorials/scripting/gdscript/gdscript_format_string.html
	pass
