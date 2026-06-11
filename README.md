# GIS Application

A lightweight Qt-based GIS viewer and processing tool for working with ESRI Shapefiles (`.shp`) and MapInfo TAB (`.tab`) datasets.

The application provides:

- GIS file loading
- Coordinate conversion
- Polygon visualization
- Interactive map navigation
- Rectangle-based clipping
- Trajectory selection and measurement
- Geospatial entity inspection

---

# Features

## Supported Formats

### ESRI Shapefile

- `.shp`
- Associated DBF attribute data

Implemented through:

- `GisShpFileReader`
- Embedded `shapelib`

### MapInfo TAB

Implemented through:

- `GisTabFileReader`
- Embedded MITAB/GDAL-style components

---

## Interactive Map Viewer

The application renders GIS polygons using the Qt Graphics View framework.

Capabilities:

- Zoom using mouse wheel
- Pan by dragging
- Fit map to viewport
- Restore original map after clipping
- Coordinate inspection

Rendering is performed using:

- `QGraphicsScene`
- `QGraphicsPolygonItem`
- `QGraphicsView`

---

## Coordinate Conversion

The application contains an abstraction layer for coordinate transformations.

### Interface

```cpp
class GisCoordinatesConverterInterface
```

### Default Implementation

```cpp
class GisCoordinatesConverterSimple
```

Supports:

- Geographic coordinate transformation
- Reverse transformation
- Custom map center definition

The user can specify:

- Center longitude
- Center latitude

and the map is reprojected around that point.

---

## Polygon Clipping

The application allows the user to select a rectangular clipping area and cut all loaded polygons against it.

### Workflow

1. Load GIS dataset
2. Switch to Clipping Mode
3. Select clipping rectangle
4. Apply clipping
5. Display clipped geometry

### Library Used

The clipping operation is implemented using:

- Clipper Library

Embedded source:

```text
clipper/
```

---

## Trajectory Selection

A trajectory can be interactively defined on top of the loaded map.

### Workflow

1. Switch to Trajectory Mode
2. Select start point
3. Select end point
4. Visualize the trajectory line

Displayed objects:

- Start marker
- End marker
- Connecting line

Useful for:

- Route inspection
- Distance estimation
- GIS feature analysis

---

# Architecture

## High-Level Structure

```text
+---------------------+
|     MainWidget      |
+----------+----------+
           |
           v
+---------------------+
| GisFileReader       |
+----------+----------+
           |
   +-------+-------+
   |               |
   v               v
Shapefile      TAB Reader
 Reader
```

Coordinate conversion is applied through a decorator:

```text
GisFileReader
       |
       v
GisFileReaderConvertDecorator
       |
       v
Coordinate Converter
```

---

# Core Components

## MainWidget

Main application window.

Responsibilities:

- User interaction
- File loading
- Map rendering
- Clipping control
- Trajectory control
- Coordinate conversion management

Key modes:

```cpp
ModeTrajectorySelecting
ModeMapClipping
```

---

## GisFileReader

Abstract base class for all GIS readers.

Responsibilities:

- Open GIS files
- Store loaded entities
- Track map boundaries
- Support clipping
- Restore original geometry

Main data:

```cpp
std::list<GisEntity> entities_;
```

---

## GisShpFileReader

Reads ESRI Shapefiles.

Uses:

- SHP geometry
- DBF attributes

Extracts:

- Polygon geometry
- Entity metadata
- Bounding information

---

## GisTabFileReader

Reads MapInfo TAB datasets.

Responsibilities:

- Open TAB files
- Read geometry
- Read feature attributes
- Calculate map bounds

---

## GisFileReaderConvertDecorator

Decorator that transparently converts coordinates after loading.

Advantages:

- Keeps readers independent of projection logic
- Allows converter replacement
- Simplifies rendering pipeline

Pattern used:

```text
Decorator Pattern
```

---

## GisEntity

Represents a GIS feature.

Contains:

### Geometry

```cpp
std::list<GAPoint>
```

### Attributes

```cpp
std::list<GisField>
```

Examples:

- Administrative regions
- Parcels
- Lakes
- Roads
- Any polygonal GIS object

---

## GisField

Represents an attribute field.

Typical examples:

| Field | Value |
|---------|---------|
| NAME | Minsk |
| TYPE | City |
| ID | 123 |

---

## GAPoint

Basic geometric point representation.

Used throughout the application for:

- GIS coordinates
- Converted coordinates
- Clipping geometry
- Trajectory endpoints

---

## GAVector

Vector mathematics utility.

Provides geometric calculations used by:

- Coordinate processing
- Map operations
- Geometry manipulation

---

## GAUtils

Collection of helper mathematical and geometric functions.

---

# Design Patterns

## Factory Pattern

Reader creation is centralized through:

```cpp
gisCreateGisFileReader()
```

Automatically selects:

- `GisShpFileReader`
- `GisTabFileReader`

based on file extension.

---

## Decorator Pattern

Implemented by:

```cpp
GisFileReaderConvertDecorator
```

Adds coordinate conversion functionality without modifying reader implementations.

---

## Strategy Pattern

Coordinate conversion is abstracted behind:

```cpp
GisCoordinatesConverterInterface
```

Different conversion algorithms can be plugged in transparently.

---

# Third-Party Libraries

## Qt

Used for:

- GUI
- Event handling
- Rendering
- Graphics scene management

Required modules:

- QtCore
- QtGui
- QtWidgets
- QtSvg

---

## Clipper

Used for:

- Polygon clipping
- Geometric intersection operations

Location:

```text
clipper/
```

---

## Shapelib

Used for:

- Reading ESRI Shapefiles
- Reading DBF attribute tables

Location:

```text
shapelib/
```

---

## Coordinate Conversion Module

Location:

```text
coordConvert/
```

Provides projection and coordinate transformation support.

---

# Build Requirements

## Compiler

- C++17 compatible compiler

## Build System

- CMake 3.16+

## Dependencies

- Qt 5

---

# Build

```bash
mkdir build
cd build

cmake ..
cmake --build .
```

---

# Typical Workflow

## Viewing a Map

1. Start application
2. Open `.shp` or `.tab` file
3. Map is loaded and rendered
4. Zoom and inspect geometry

---

## Clipping a Map

1. Load dataset
2. Select **Clipping Mode**
3. Mark clipping rectangle
4. Apply clipping
5. Review clipped geometry

---

## Defining a Trajectory

1. Load dataset
2. Select **Trajectory Mode**
3. Click start point
4. Click end point
5. Trajectory line appears

---

# Project Structure

```text
gis-application/
│
├── clipper/
│   └── Polygon clipping library
│
├── coordConvert/
│   └── Coordinate conversion module
│
├── shapelib/
│   └── ESRI Shapefile support
│
├── gapoint.*
├── gavector.*
├── gautils.*
│
├── gisentity.*
├── gisfield.*
│
├── gisfilereader.*
├── gisfilereaderconvertdecorator.*
├── gisfilereaders.*
│
├── gisshpfilereader.*
├── gistabfilereader.*
│
├── mainwidget.*
├── main.cpp
│
└── CMakeLists.txt
```

# Summary

This project is a compact desktop GIS application written in C++ and Qt. It combines GIS file parsing, coordinate transformation, polygon clipping, and interactive visualization into a modular architecture built around factory, strategy, and decorator design patterns. The application is particularly suited for educational GIS projects, lightweight geospatial analysis tools, and custom map-processing workflows.
