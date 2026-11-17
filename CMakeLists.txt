# One source tree, three independent simulation executable targets.
cmake_minimum_required(VERSION 3.16...3.27)
project(Geant4Apps LANGUAGES CXX)

find_package(Geant4 11.3.2 REQUIRED ui_all vis_all)
include(${Geant4_USE_FILE})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Explicit lists prevent the applications' same-named classes from mixing.

add_executable(dnaphysics
    dnaphysics.cc
    include/dna_ActionInitialization.hh
    include/dna_DetectorConstruction.hh
    include/dna_DetectorMessenger.hh
    include/dna_PhysicsList.hh
    include/dna_PrimaryGeneratorAction.hh
    include/dna_RunAction.hh
    include/dna_SteppingAction.hh
    include/dna_TrackingAction.hh
    src/dna_ActionInitialization.cc
    src/dna_DetectorConstruction.cc
    src/dna_DetectorMessenger.cc
    src/dna_PhysicsList.cc
    src/dna_PrimaryGeneratorAction.cc
    src/dna_RunAction.cc
    src/dna_SteppingAction.cc
    src/dna_TrackingAction.cc
)
target_include_directories(dnaphysics PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(dnaphysics PRIVATE ${Geant4_LIBRARIES})

add_executable(molecular
    molecular.cc
    include/molecular_ActionInitialization.hh
    include/molecular_AnalysisManager.hh
    include/molecular_AnalysisMessenger.hh
    include/molecular_BoxChromosome.hh
    include/molecular_ChemistryList.hh
    include/molecular_ChromosomeFactory.hh
    include/molecular_ChromosomeHit.hh
    include/molecular_ChromosomeMapper.hh
    include/molecular_ChromosomeMessenger.hh
    include/molecular_CylindricalChromosome.hh
    include/molecular_DNAGeometry.hh
    include/molecular_DNAGeometryMessenger.hh
    include/molecular_DNAHashing.hh
    include/molecular_DNAHit.hh
    include/molecular_DNAWorld.hh
    include/molecular_DamageModel.hh
    include/molecular_DamageModelMessenger.hh
    include/molecular_DetectorConstruction.hh
    include/molecular_DetectorMessenger.hh
    include/molecular_EllipticalChromosome.hh
    include/molecular_EventAction.hh
    include/molecular_IRTDamageReactionModel.hh
    include/molecular_MoleculeList.hh
    include/molecular_OctreeNode.hh
    include/molecular_ParallelWorldPhysics.hh
    include/molecular_PhysicsList.hh
    include/molecular_PlacementVolumeInfo.hh
    include/molecular_PrimaryGeneratorAction.hh
    include/molecular_PrimaryGeneratorMessenger.hh
    include/molecular_PrimaryGeneratorSource.hh
    include/molecular_PrimaryGeneratorSourceGRASCSV.hh
    include/molecular_RodChromosome.hh
    include/molecular_RunAction.hh
    include/molecular_SphericalChromosome.hh
    include/molecular_StackingAction.hh
    include/molecular_SteppingAction.hh
    include/molecular_TimeStepAction.hh
    include/molecular_UtilityFunctions.hh
    include/molecular_VirtualChromosome.hh
    src/molecular_ActionInitialization.cc
    src/molecular_AnalysisManager.cc
    src/molecular_AnalysisMessenger.cc
    src/molecular_BoxChromosome.cc
    src/molecular_ChemistryList.cc
    src/molecular_ChromosomeFactory.cc
    src/molecular_ChromosomeHit.cc
    src/molecular_ChromosomeMapper.cc
    src/molecular_ChromosomeMessenger.cc
    src/molecular_CylindricalChromosome.cc
    src/molecular_DNAGeometry.cc
    src/molecular_DNAGeometryMessenger.cc
    src/molecular_DNAHashing.cc
    src/molecular_DNAHit.cc
    src/molecular_DNAWorld.cc
    src/molecular_DamageModel.cc
    src/molecular_DamageModelMessenger.cc
    src/molecular_DetectorConstruction.cc
    src/molecular_DetectorMessenger.cc
    src/molecular_EllipticalChromosome.cc
    src/molecular_EventAction.cc
    src/molecular_IRTDamageReactionModel.cc
    src/molecular_OctreeNode.cc
    src/molecular_ParallelWorldPhysics.cc
    src/molecular_PhysicsList.cc
    src/molecular_PlacementVolumeInfo.cc
    src/molecular_PrimaryGeneratorAction.cc
    src/molecular_PrimaryGeneratorMessenger.cc
    src/molecular_PrimaryGeneratorSourceGRASCSV.cc
    src/molecular_RodChromosome.cc
    src/molecular_RunAction.cc
    src/molecular_SphericalChromosome.cc
    src/molecular_StackingAction.cc
    src/molecular_SteppingAction.cc
    src/molecular_TimeStepAction.cc
    src/molecular_UtilityFunctions.cc
    src/molecular_VirtualChromosome.cc
)
target_include_directories(molecular PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(molecular PRIVATE ${Geant4_LIBRARIES})

add_executable(chem4
    chem4.cc
    include/chem4_ActionInitialization.hh
    include/chem4_CommandLineParser.hh
    include/chem4_DetectorConstruction.hh
    include/chem4_PhysicsList.hh
    include/chem4_PrimaryGeneratorAction.hh
    include/chem4_PrimaryKiller.hh
    include/chem4_Run.hh
    include/chem4_RunAction.hh
    include/chem4_ScoreSpecies.hh
    include/chem4_StackingAction.hh
    include/chem4_TrackingAction.hh
    src/chem4_ActionInitialization.cc
    src/chem4_CommandLineParser.cc
    src/chem4_DetectorConstruction.cc
    src/chem4_PhysicsList.cc
    src/chem4_PrimaryGeneratorAction.cc
    src/chem4_PrimaryKiller.cc
    src/chem4_Run.cc
    src/chem4_RunAction.cc
    src/chem4_ScoreSpecies.cc
    src/chem4_StackingAction.cc
    src/chem4_TrackingAction.cc
)
target_include_directories(chem4 PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(chem4 PRIVATE ${Geant4_LIBRARIES})

# Preserve the example's optional ROOT GUI plotting program.
find_package(ROOT COMPONENTS Gui QUIET)
if(ROOT_FOUND AND ROOT_USE_FILE)
    add_subdirectory(analysis/chem4_plot)
    add_dependencies(plotG chem4)
    # ROOT's interpreted GUI callbacks must resolve symbols from the executable.
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        target_link_options(plotG PRIVATE -rdynamic)
    endif()
else()
    message(STATUS "chem4 plotG disabled: ROOT with GUI was not found; chem4 can still write ROOT output.")
endif()

set_target_properties(dnaphysics molecular chem4 PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Keep vis.mac and all relative run/output paths separate for each program.
foreach(app IN ITEMS dnaphysics molecular chem4)
    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/run/${app}")
    file(GLOB_RECURSE macros CONFIGURE_DEPENDS
        RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/macros/${app}"
        "${CMAKE_CURRENT_SOURCE_DIR}/macros/${app}/*")
    foreach(macro IN LISTS macros)
        # Copy only chem4 simulation macros into its run directory.
        if(app STREQUAL "chem4" AND NOT macro MATCHES "\\.(in|mac)$")
            continue()
        endif()
        configure_file("macros/${app}/${macro}" "run/${app}/${macro}" COPYONLY)
    endforeach()
endforeach()

# Reuse the existing large dataset; do not download it during configuration.
set(MOLECULAR_GEOMETRY_DIR
    "$ENV{HOME}/Applications/GEANT4/molecularDNA-build/geometries"
    CACHE PATH "Existing molecularDNA geometries directory")
if(IS_DIRECTORY "${MOLECULAR_GEOMETRY_DIR}")
    file(CREATE_LINK "${MOLECULAR_GEOMETRY_DIR}"
        "${CMAKE_BINARY_DIR}/run/molecular/geometries" SYMBOLIC RESULT link_result)
    if(NOT link_result STREQUAL "0")
        message(FATAL_ERROR "Could not link molecularDNA geometry data: ${link_result}")
    endif()
else()
    message(STATUS "To run molecularDNA geometry macros, set MOLECULAR_GEOMETRY_DIR to an existing dataset.")
endif()
