# Blender Addon Identifier Info
bl_info = {
    "name": "King Render Engine",
    "author" : "Nicholas Tuttle",
    "description" : "King Render Engine",
    "blender" : (3, 2, 0),
    "category": "Render"
}

# Support 'reload' case.
if "bpy" in locals():
    import importlib
    if "KingRenderEngine" in locals():
        importlib.reload(KingRenderEngine)

from . import KingRenderEngine

def register():
    KingRenderEngine.register()
    

def unregister():
    KingRenderEngine.unregister()