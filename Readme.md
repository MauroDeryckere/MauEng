# 3D Game Engine Project
[![Build Debug](https://github.com/MauroDeryckere/MauEng/actions/workflows/build-debug.yml/badge.svg)](https://github.com/MauroDeryckere/MauEng/actions/workflows/build-debug.yml)
[![Build Release](https://github.com/MauroDeryckere/MauEng/actions/workflows/build-release.yml/badge.svg)](https://github.com/MauroDeryckere/MauEng/actions/workflows/build-release.yml)

## Table of Contents
- [3D Game Engine Project](#3d-game-engine-project)
  - [Table of Contents](#table-of-contents)
  - [Introduction](#introduction)
  - [Demo](#demo)
  - [Core](#core)
	- [Debugging - Logging](#debugging---logging)
	- [Debugging - Asserts](#debugging---asserts)
	- [Event System](#event-system)
	- [Timer Manager](#timer-manager)
	- [UUID](#uuid)
	- [Profiling](#profiling)
	- [Libraries](#libraries)
  - [Engine](#engine)
	- [Input System](#input-system)
	- [Component System](#component-system)
	- [Renderer](#renderer)
	  - [Coordinate System](#coordinate-system)
	  - [Pipeline Overview](#Pipeline-Overview)
	  - [Features](#features)
	  - [Debug Rendering](#debug-rendering)
  - [Features I want to add inn the near future](#features-i-want-to-add-inn-the-near-future)
	  - [In the further future](#in-the-further-future)

## Introduction

## Demo
A small demo scene is included in the project, it showcases most of the features of the engine. The demo scene can be found in Game/DemoScene.h and .cpp<br>

**Watch the Demo Video (click):** </br>  
[![Watch the demo](https://img.youtube.com/vi/kbcXu0AuIoM/0.jpg)](https://www.youtube.com/watch?v=kbcXu0AuIoM)

**Config**<br>
Change the EDemo m_Demo; in the header to a different enum value to view different scenes.<br>
Config.cmake: build settings<br>
Renderer/Config/VulkanConfigs: buffer sizes, extensions <br>
Core/Public/Config/EngineConfig.h: macros, general settings<br>

**Key binds**<br>
SPACE: Output the keybinds<br>
F1: Profile<br>
F2: Toggle light debug render (display spheres for point light, arrow for dir light)<br>
F3: Toggle light mode - point light only, dir light only, both<br>
F4: Toggle shadows<br>
F5: Lower light intensity <br>
F6: Up light intensity<br>
F7: Toggle rotation<br>
F8: Toggle Debug render mode<br>
F9: Randomize light color<br>
F10: Toggle cam settings<br>
F11: Toggle tone mapper<br>
<br>
E: Lower custom exposure (used in custom exposure mode)<br>
R: Higher custom exposure (used in custom exposure mode)<br>
<br>
Mouse movement + Left Mouse Button Held: Cam rotation<br>
Arrows/WASD: Cam movement<br>
Control: Sprint <br>
<br>
## Core
The engine's core contains basic functionality used by the engine (and the game). It includes debugging tools, a timer manager, an event system, a UUID generator, and profiling tools.

### Debugging -Logging
The logger can log to the console and a file using different log priority levels and categories. The priority of logging can be adjusted to skip logging all levels below the set level; this priority adjustment can also be done per log category. The colors of the console logs are configurable.

The file logging has a configurable file size, before it rotates to the next file. Currently, it simply keeps a single backup. If a full backup is stored and the new rotation happens, the backup is overwritten with the new file. The file also contains the log level more clearly and is timestamped.

```cpp
// Logging can be done using the LOG macro or using the specific _Priority level macro.
ME_LOG(MauCor::LogPriority::Error, MauCor::LogCategory::Game,"test {}", 1000);
ME_LOG_ERROR(MauCor::LogCategory::Game, "TEST");

// Creating a custom log category can be done in the following way:
// Define the category in a single cpp file
DEFINE_LOG_CATEGORY(TestLogCategory)
DEFINE_LOG_CATEGORY(TestLogCategory2, Warn) // If you wish to set a log priority level for the category

TestLogCategory2.SetPriority(Debug); // Or change it afterward

// In the header / cpp files where you wish to use the log category
DECLARE_LOG_CATEGORY_EXTERN(TestLogCategory)
```

Example of console logging (Renderer category, info & trace log level)
![Screenshot](docs/LoggerExample.png)

Example of file logging (contains time stamp, category & log level)
![Screenshot](docs/LoggerFileExample.png)

### Debugging - Asserts
- Assert only triggers in debug, the message is an optional parameter that will be logged to the console / file logger.
- Check triggers in all builds, message is an optional parameter that will be logged to the console / file logger.
- Verify logs a warning in release builds, fails in debug. This means code in the macro is still executed in release. Message is an optional parameter that will be logged to the console / file logger.

| Macro       | Build Type  | Fails on Condition? | Removed in Release? | Use Case									|
|-------------|-------------|---------------------|---------------------|-------------------------------------------|
| ME_ASSERT	  | Debug-only  | Yes (fatal)         | Yes                 | Catching programmer errors				|
| ME_CHECK    | All builds  | Yes (fatal)         | No                  | Critical runtime invariants				|
| ME_VERIFY   | All builds* | No (log only)*      | No*                 | Validate logic without halting execution  |

```cpp
// Some assert examples
ME_ASSERT(std::filesystem::exists("Path123.txt"));
ME_CHECK(pRenderer, "Renderer must be valid");
ME_VERIFY(CalculateAndValidatePath(), "Path must be valid");
```

### Event System
The event system is simple to use. The only requirement is storing a Delegate and creating an event class.
*Note: A MauCor::Delegate<> is implicitly the same type as MauCor::Delegate<void>*
```cpp
struct TestEvent
{
	int i = 10;
};

class Scene
{
	MauCor::Delegate<TestEvent> m_DelegateTest{};
	MauCor::Delegate<> m_DelegateVoidTest{};
};
```

Broadcasting events and listening to events is also fairly simple, but it comes with different options, as you may want an immediate broadcast (which calls the corresponding function immediately when the event is broadcast). Or a delayed broadcast (which calls the corresponding function at the beginning of the next frame when the event is broadcast).

Similar to broadcasting, unsubscribes can be done immediately and delayed as well. The default here is to do it delayed, which prevents issues where you may unsubscribe, but there's still a lingering function call, resulting in nullptr or invalid ptr usage.

```cpp
// Subscribe to an event
m_DelegateTest += MauCor::Bind(&DemoScene::OnDelegate, this);

auto const& handle{ m_DelegateTest += MauCor::Bind<TestEvent>
(
	[this](TestEvent const& event) { OnDelegate(event); }
) };

m_DelegateTest.Get()->Subscribe([this](TestEvent const& event) { OnDelegate(event); }, this);

// Broadcast an event (the default is the immediate broadcast)
m_DelegateTest.Get()->QueueBroadcast(event);
m_DelegateTest.QueueBroadcast(event);
m_DelegateTest << event;
// Do an immediate broadcast:
m_DelegateTest.Get()->Broadcast(event);
m_DelegateTest.Broadcast(event);
m_DelegateTest < event;

// Unsubscribe from an event
m_DelegateTest /= this; // Immediate
m_DelegateTest.UnSubscribeImmediate(handle02.owner);
m_DelegateTest.UnSubscribeAllByOwnerImmediate(this);

m_DelegateTest -= handle; // Delayed
m_DelegateTest.UnSubscribe(handle02.owner);

// The function called in this example
void DemoScene::OnDelegate(TestEvent const& event)
{
	ME_LOG_DEBUG(LogGame, "Event test: {}", event.i);
}
```

### Timer Manager
TODO

### UUID
Small custom UUID library that generates a unique identifier for each object. It is used to identify objects in the engine, such as entities, components, and resources.</br></br>
[View UUID Library on GitHub](https://github.com/MauroDeryckere/UUID)

### Profiling
The engine has 2 available profilers, a very barebones profiler that simply parses to a .json file and can be uploaded to chrome://tracing/. The other profiler is an integration of the Optick library and provides a lot more information if required.

Profiling requires 2 steps. 
1. add the macros in all the functions & scopes you wish to profile: 
```cpp
// Examples
void MySleepFunc() {
	// Do stuff
	{
		ME_PROFILE_SCOPE("Main Thread Sleep");
		Sleep();
	}
}

void Render(){
	ME_PROFILE_FUNCTION();
}

```

2. Press F1 to start profiling & upload it to the exe or chrome://tracing/.

Profiling only happens when it is enabled in the Config.cmake file.

### Libraries
Core libraries used all over the engine, that the user may or may not also need access to: SDL, fmt, glm and Optick.

## Engine

## Input System
TODO

## Component System
The engine currently uses a wrapper around entts component system, it supports almost all functions entt offers.

## Renderer
### Coordinate System
In this project, we use a right-handed 3D coordinate system with the following conventions:
**X-axis:** Represents the horizontal direction.
- Positive X moves to the right.
- Negative X moves to the left.

**Y-axis:** Represents the vertical direction (with Y-up convention).
- Positive Y moves upward.
- Negative Y moves downward.

**Z-axis**: Represents the depth direction.
- Positive Z moves forward (towards the camera's view).
- Negative Z moves backward (away from the camera's view).

### Pipeline Overview
![Screenshot](docs/Pipeline.png)

### Features
- Instanced Rendering<br>
As a test I loaded a mesh with 100 000 instances. The mesh is a simple gun and has 1425 indices and 311 vertices. This runs very smoothly on my hardware (RTX 3060, 60 FPS cap but main thread was sleeping for +/-10ms and GPU had a lot of room left)
![Screenshot](docs/ZoomedOutInstances.png)
![Screenshot](docs/ZoomedInInstances.png)

- Bindless (indirect) Rendering<br>
The renderer uses a global index and vertex buffer, draw commands are batched and issued using vkCmdDrawIndexedIndirect. Textures are in a descriptor array.

- Deferred rendering<br>

- Depth prepass<br>
Reduce overdaw by doing a depth prepass.

- Dynamic rendering<br>

- Mesh & material support (loading a material from a file)<br>
Assimp is integrated, and all formats supported by Assimp can be used to load meshes & materials. Meshes are split up in submeshes, these submeshes are then instanced.
Default and invalid materials are used to prevent branching on the GPU.

- Lighting & Material<br>
 
- Tone map & Exposure<br>
![Screenshot](docs/FlightHelmetExample.png)


### Debug Rendering
Easy to use API for debug rendering. See. Debug rendering demo.

```cpp
void GameScene::Tick()
{
	Scene::Tick();

	// start, end, colour
	DEBUG_RENDERER.DrawLine({0, 0,0 },  {0, 100, 100} );
	DEBUG_RENDERER.DrawLine({-10 , 10, -10}, {10, 10, 10}, { 0, 1, 0});

	// center, radius, colour, segmenets (per circle), layers
	DEBUG_RENDERER.DrawSphereComplex({20,20,20}, 20.f, { 1, 1, 1 }, 24, 10);
}
```
**Watch the Demo Video (click):** </br>  
[![Watch the demo](https://img.youtube.com/vi/oeMaRM3xfdg/0.jpg)](https://www.youtube.com/watch?v=oeMaRM3xfdg)

## Features I want to add in the near future
- Image based lighting (skybox)
- Auto exposure
- GPU frustrum culling
- Optimized scene AABB calculation for shadow maps
- Soft shadows
- ImGUI integration

### In the further future
- Animations
- Different Post processing effects

