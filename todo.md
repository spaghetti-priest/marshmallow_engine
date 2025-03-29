Marshmallow Engine Ideas and Todo Compendium:

Engine Architecutre idea research:
	Doom-3-BFG engine:
	Quake 2 engine:
	Source 2 engine:

Editor Architecture ideas:
	Trenchbroom
	Blender CSG tools
	Xcode Metal debugger visualizer

Game Ideas:
	Im thinking about a small isometric game similar to Fallout or Planescape torment. In terms of visuals
	Im very inspiried by the look of games like Dreams or The last guardian or Disco elysium and the claymation aspects of
	fallout models. hmmm....
	I moving towards the look towards claymation esque visuals. SDF? this might be because of my claymation aspirations hmm...

I might use SDL2 and Vulkan for ease of setup and use. Imgui for the UI. Im primiarily interested in the construction of the engine, the game is secondary.

Possible architectual roadmap:
- Tilemap based? In fallout 1 the map and light operates on hexagonal bounds, possibly inspired by the hexagonal nature of some tabletop games.
- I like the CSG and mesh tools in source 2 sdk.
- Dota 2's source 2 tile editor is nice
- Using 3D models like Dota 2
- Maybe orthographic view?
- Point and click for movement but the camera can move past player position

.todo:
	- Implement Djikstras algorithm
	- makefile for MACOS and linux

.completed:
	[X] Build system
		[x] build.bat for Win32
	[x] Basic Mouse input
	[x] Basic Keyboard input
	[x] Graphical output
	[x] Timers
	[?] Frame rate
		[] Frame rate independent movement
	[x] Game memory
	[x] Memory arena
		[] Dynamic scratch buffers
		[] Alignment of memory arena
	[X] File platform
		[x] Read file
		[x] Write file
		[?] Directory
	[x] Tilemapping
		[x] Mouse should highlight over tile
		[x] Able to aquire tilevalue within the tilemap
		[x] Create a tilemap structure?
		[x] Player updates position after double click on a tile
		[] Tilemap should extend beyond the current screen space
	[] A* pathfindin
		[X] Tile neighbors (Neighbors represent the edges of the Tilemap, if you were to consider the tilemap as if it were a graph)
		[X] Heuristic functions implemented: Euclidean, Manhattan and Chebyshev distances
		[X] Basic distance finding search using mouse cursor
		[x] Player can traverse the grid through the greed search function, using right bracket
			[x] Turn this into an animated movement
		[] Implement Djikstras algorithm
		[] Maneuver past untraversable gridtiles
	[] Turn based game loop
	[] Game map
		[x] Tilevalue system which determines whether the tile is traversable or a boundry
		[?] These tilevalues should be set by the user code.
		[] At runtime the user should be able to change the tilevalue of a tile.
		[] Game should read a map (online or offline) based structure then render it
	[] Editor mode:
		[X] Be able to either switch to editor mode via a key or a UI option
		[] At runtime the user should be able to change the tilevalue of a tile when in editor mode.
		[] At runtime the user should be able to create a tilemap when in editor mode
		[] Editor mode should be able pause the runtime
	[] Camera
		[] Basic camera function
		[] Camera should extend beyond the player pos
	[] Sound engine layer
	[x] Gamestate input
		[x] Left click changes the color of the tile
		[x] Double left click changes the tile again
	[] Entities / Entity management
		[x] Player entity
		[] Standing entity which can prompt a certain action like dialogue or something like pulling a lever
	[x] Vulkan layer
		[] Texture loading
			[X] stbi_image load
			[] vulkan image view
		[] Text loading and stuff
			[] Implement STB_Truetype in Vulkan
		[] Tile render
			[] Draw quad
		[] Meshes
			[] OBJ
			[] GLTF
		[] Imgui or custom UI(?)
		[] Camera
			[] Basic camera function
			[] Camera should extend beyond the player pos

	[] AI
	[] Gamepad input (?)
	[] Multithreading (?)