# Blender Addon Identifier Info
bl_info = {
    "name": "Cosmic Render Engine",
    "author" : "Nicholas Tuttle",
    "description" : "Cosmic Render Engine",
    "blender" : (3, 5, 0),
    "category": "Render"
}

# Support 'reload' case.
if "bpy" in locals():
    import importlib
    if "CosmicRenderEngine" in locals():
        importlib.reload(CosmicRenderEngine)

from . import CosmicRenderEngine

def register():
    CosmicRenderEngine.register()
    

def unregister():
    CosmicRenderEngine.unregister()