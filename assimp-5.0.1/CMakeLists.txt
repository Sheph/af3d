SET( HEADER_PATH ../include/assimp )

SET( COMPILER_HEADERS
  ${HEADER_PATH}/Compiler/pushpack1.h
  ${HEADER_PATH}/Compiler/poppack1.h
  ${HEADER_PATH}/Compiler/pstdint.h
)

SET( PUBLIC_HEADERS
  ${HEADER_PATH}/anim.h
  ${HEADER_PATH}/aabb.h
  ${HEADER_PATH}/ai_assert.h
  ${HEADER_PATH}/camera.h
  ${HEADER_PATH}/color4.h
  ${HEADER_PATH}/color4.inl
  ${HEADER_PATH}/config.h
  ${HEADER_PATH}/defs.h
  ${HEADER_PATH}/Defines.h
  ${HEADER_PATH}/cfileio.h
  ${HEADER_PATH}/light.h
  ${HEADER_PATH}/material.h
  ${HEADER_PATH}/material.inl
  ${HEADER_PATH}/matrix3x3.h
  ${HEADER_PATH}/matrix3x3.inl
  ${HEADER_PATH}/matrix4x4.h
  ${HEADER_PATH}/matrix4x4.inl
  ${HEADER_PATH}/mesh.h
  ${HEADER_PATH}/pbrmaterial.h
  ${HEADER_PATH}/postprocess.h
  ${HEADER_PATH}/quaternion.h
  ${HEADER_PATH}/quaternion.inl
  ${HEADER_PATH}/scene.h
  ${HEADER_PATH}/metadata.h
  ${HEADER_PATH}/texture.h
  ${HEADER_PATH}/types.h
  ${HEADER_PATH}/vector2.h
  ${HEADER_PATH}/vector2.inl
  ${HEADER_PATH}/vector3.h
  ${HEADER_PATH}/vector3.inl
  ${HEADER_PATH}/version.h
  ${HEADER_PATH}/cimport.h
  ${HEADER_PATH}/importerdesc.h
  ${HEADER_PATH}/Importer.hpp
  ${HEADER_PATH}/DefaultLogger.hpp
  ${HEADER_PATH}/ProgressHandler.hpp
  ${HEADER_PATH}/IOStream.hpp
  ${HEADER_PATH}/IOSystem.hpp
  ${HEADER_PATH}/Logger.hpp
  ${HEADER_PATH}/LogStream.hpp
  ${HEADER_PATH}/NullLogger.hpp
  ${HEADER_PATH}/cexport.h
  ${HEADER_PATH}/Exporter.hpp
  ${HEADER_PATH}/DefaultIOStream.h
  ${HEADER_PATH}/DefaultIOSystem.h
  ${HEADER_PATH}/ZipArchiveIOSystem.h
  ${HEADER_PATH}/SceneCombiner.h
  ${HEADER_PATH}/fast_atof.h
  ${HEADER_PATH}/qnan.h
  ${HEADER_PATH}/BaseImporter.h
  ${HEADER_PATH}/Hash.h
  ${HEADER_PATH}/MemoryIOWrapper.h
  ${HEADER_PATH}/ParsingUtils.h
  ${HEADER_PATH}/StreamReader.h
  ${HEADER_PATH}/StreamWriter.h
  ${HEADER_PATH}/StringComparison.h
  ${HEADER_PATH}/StringUtils.h
  ${HEADER_PATH}/SGSpatialSort.h
  ${HEADER_PATH}/GenericProperty.h
  ${HEADER_PATH}/SpatialSort.h
  ${HEADER_PATH}/SkeletonMeshBuilder.h
  ${HEADER_PATH}/SmoothingGroups.h
  ${HEADER_PATH}/SmoothingGroups.inl
  ${HEADER_PATH}/StandardShapes.h
  ${HEADER_PATH}/RemoveComments.h
  ${HEADER_PATH}/Subdivision.h
  ${HEADER_PATH}/Vertex.h
  ${HEADER_PATH}/LineSplitter.h
  ${HEADER_PATH}/TinyFormatter.h
  ${HEADER_PATH}/Profiler.h
  ${HEADER_PATH}/LogAux.h
  ${HEADER_PATH}/Bitmap.h
  ${HEADER_PATH}/XMLTools.h
  ${HEADER_PATH}/IOStreamBuffer.h
  ${HEADER_PATH}/CreateAnimMesh.h
  ${HEADER_PATH}/irrXMLWrapper.h
  ${HEADER_PATH}/BlobIOSystem.h
  ${HEADER_PATH}/MathFunctions.h
  ${HEADER_PATH}/Macros.h
  ${HEADER_PATH}/Exceptional.h
  ${HEADER_PATH}/ByteSwapper.h
)

SET( Core_SRCS
  Common/Assimp.cpp
)

SET( Logging_SRCS
  ${HEADER_PATH}/DefaultLogger.hpp
  ${HEADER_PATH}/LogStream.hpp
  ${HEADER_PATH}/Logger.hpp
  ${HEADER_PATH}/NullLogger.hpp
  Common/Win32DebugLogStream.h
  Common/DefaultLogger.cpp
  Common/FileLogStream.h
  Common/StdOStreamLogStream.h
)

SET( Common_SRCS
  Common/BaseImporter.cpp
  Common/BaseProcess.cpp
  Common/BaseProcess.h
  Common/Importer.h
  Common/ScenePrivate.h
  Common/PostStepRegistry.cpp
  Common/ImporterRegistry.cpp
  Common/DefaultProgressHandler.h
  Common/DefaultIOStream.cpp
  Common/DefaultIOSystem.cpp
  Common/ZipArchiveIOSystem.cpp
  Common/PolyTools.h
  Common/Importer.cpp
  Common/IFF.h
  Common/SGSpatialSort.cpp
  Common/VertexTriangleAdjacency.cpp
  Common/VertexTriangleAdjacency.h
  Common/SpatialSort.cpp
  Common/SceneCombiner.cpp
  Common/ScenePreprocessor.cpp
  Common/ScenePreprocessor.h
  Common/SkeletonMeshBuilder.cpp
  Common/SplitByBoneCountProcess.cpp
  Common/SplitByBoneCountProcess.h
  Common/StandardShapes.cpp
  Common/TargetAnimation.cpp
  Common/TargetAnimation.h
  Common/RemoveComments.cpp
  Common/Subdivision.cpp
  Common/scene.cpp
  Common/Bitmap.cpp
  Common/Version.cpp
  Common/CreateAnimMesh.cpp
  Common/simd.h
  Common/simd.cpp
)

SET( CApi_SRCS
  CApi/CInterfaceIOWrapper.cpp
  CApi/CInterfaceIOWrapper.h
)

SET( STEPParser_SRCS
  Importer/STEPParser/STEPFileReader.h
  Importer/STEPParser/STEPFileReader.cpp
  Importer/STEPParser/STEPFileEncoding.cpp
  Importer/STEPParser/STEPFileEncoding.h
)

# if this variable is set to TRUE, the user can manually disable importers by setting
# ASSIMP_BUILD_XXX_IMPORTER to FALSE for each importer
# if this variable is set to FALSE, the user can manually enable importers by setting
# ASSIMP_BUILD_XXX_IMPORTER to TRUE for each importer
OPTION(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT "default value of all ASSIMP_BUILD_XXX_IMPORTER values" TRUE)

# macro to add the CMake Option ADD_ASSIMP_IMPORTER_<name> which enables compile of loader
# this way selective loaders can be compiled (reduces filesize + compile time)
MACRO(ADD_ASSIMP_IMPORTER name)
  IF (ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT)
    set(ASSIMP_IMPORTER_ENABLED TRUE)
    IF (DEFINED ASSIMP_BUILD_${name}_IMPORTER AND NOT ASSIMP_BUILD_${name}_IMPORTER)
      set(ASSIMP_IMPORTER_ENABLED FALSE)
    ENDIF ()
  ELSE ()
    set(ASSIMP_IMPORTER_ENABLED ${ASSIMP_BUILD_${name}_IMPORTER})
  ENDIF ()
  IF (ASSIMP_IMPORTER_ENABLED)
    LIST(APPEND ASSIMP_LOADER_SRCS ${ARGN})
    SET(ASSIMP_IMPORTERS_ENABLED "${ASSIMP_IMPORTERS_ENABLED} ${name}")
  ELSE()
    SET(${name}_SRC "")
    SET(ASSIMP_IMPORTERS_DISABLED "${ASSIMP_IMPORTERS_DISABLED} ${name}")
    add_definitions(-DASSIMP_BUILD_NO_${name}_IMPORTER)
  ENDIF()
ENDMACRO()

# if this variable is set to TRUE, the user can manually disable exporters by setting
# ASSIMP_BUILD_XXX_EXPORTER to FALSE for each exporter
# if this variable is set to FALSE, the user can manually enable exporters by setting
# ASSIMP_BUILD_XXX_EXPORTER to TRUE for each exporter
OPTION(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT "default value of all ASSIMP_BUILD_XXX_EXPORTER values" TRUE)

# macro to add the CMake Option ADD_ASSIMP_IMPORTER_<name> which enables compile of loader
# this way selective loaders can be compiled (reduces filesize + compile time)
MACRO(ADD_ASSIMP_EXPORTER name)
  IF (ASSIMP_NO_EXPORT)
	set(ASSIMP_EXPORTER_ENABLED FALSE)
  ELSEIF (ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT)
    set(ASSIMP_EXPORTER_ENABLED TRUE)
    IF (DEFINED ASSIMP_BUILD_${name}_EXPORTER AND NOT ASSIMP_BUILD_${name}_EXPORTER)
      set(ASSIMP_EXPORTER_ENABLED FALSE)
    ENDIF ()
  ELSE ()
    set(ASSIMP_EXPORTER_ENABLED ${ASSIMP_BUILD_${name}_EXPORTER})
  ENDIF ()

  IF (ASSIMP_EXPORTER_ENABLED)
    SET(ASSIMP_EXPORTERS_ENABLED "${ASSIMP_EXPORTERS_ENABLED} ${name}")
	LIST(APPEND ASSIMP_EXPORTER_SRCS ${ARGN})
  ELSE()
    SET(ASSIMP_EXPORTERS_DISABLED "${ASSIMP_EXPORTERS_DISABLED} ${name}")
    add_definitions(-DASSIMP_BUILD_NO_${name}_EXPORTER)
  ENDIF()
ENDMACRO()

SET(ASSIMP_LOADER_SRCS "")
SET(ASSIMP_IMPORTERS_ENABLED "") # list of enabled importers
SET(ASSIMP_IMPORTERS_DISABLED "") # disabled importers list (used to print)

SET(ASSIMP_EXPORTER_SRCS "")
SET(ASSIMP_EXPORTERS_ENABLED "") # list of enabled exporters
SET(ASSIMP_EXPORTERS_DISABLED "") # disabled exporters list (used to print)

ADD_ASSIMP_IMPORTER( AMF
  AMF/AMFImporter.hpp
  AMF/AMFImporter_Macro.hpp
  AMF/AMFImporter_Node.hpp
  AMF/AMFImporter.cpp
  AMF/AMFImporter_Geometry.cpp
  AMF/AMFImporter_Material.cpp
  AMF/AMFImporter_Postprocess.cpp
)

ADD_ASSIMP_IMPORTER( 3DS
  3DS/3DSConverter.cpp
  3DS/3DSHelper.h
  3DS/3DSLoader.cpp
  3DS/3DSLoader.h
)

ADD_ASSIMP_EXPORTER( 3DS
  3DS/3DSExporter.h
  3DS/3DSExporter.cpp
)

ADD_ASSIMP_IMPORTER( AC
  AC/ACLoader.cpp
  AC/ACLoader.h
)

ADD_ASSIMP_IMPORTER( ASE
  ASE/ASELoader.cpp
  ASE/ASELoader.h
  ASE/ASEParser.cpp
  ASE/ASEParser.h
)

ADD_ASSIMP_IMPORTER( ASSBIN
  Assbin/AssbinLoader.h
  Assbin/AssbinLoader.cpp
)

ADD_ASSIMP_EXPORTER( ASSBIN
  Assbin/AssbinExporter.h
  Assbin/AssbinExporter.cpp
)

ADD_ASSIMP_EXPORTER( ASSXML
  Assxml/AssxmlExporter.h
  Assxml/AssxmlExporter.cpp
)

ADD_ASSIMP_IMPORTER( B3D
  B3D/B3DImporter.cpp
  B3D/B3DImporter.h
)

ADD_ASSIMP_IMPORTER( BVH
  BVH/BVHLoader.cpp
  BVH/BVHLoader.h
)

ADD_ASSIMP_IMPORTER( COLLADA
  Collada/ColladaHelper.h
  Collada/ColladaLoader.cpp
  Collada/ColladaLoader.h
  Collada/ColladaParser.cpp
  Collada/ColladaParser.h
)

ADD_ASSIMP_EXPORTER( COLLADA
  Collada/ColladaExporter.h
  Collada/ColladaExporter.cpp
)

ADD_ASSIMP_IMPORTER( DXF
  DXF/DXFLoader.cpp
  DXF/DXFLoader.h
  DXF/DXFHelper.h
)

ADD_ASSIMP_IMPORTER( CSM
  CSM/CSMLoader.cpp
  CSM/CSMLoader.h
)

ADD_ASSIMP_IMPORTER( HMP
  HMP/HMPFileData.h
  HMP/HMPLoader.cpp
  HMP/HMPLoader.h
  HMP/HalfLifeFileData.h
)

ADD_ASSIMP_IMPORTER( IRRMESH
  Irr/IRRMeshLoader.cpp
  Irr/IRRMeshLoader.h
  Irr/IRRShared.cpp
  Irr/IRRShared.h
)

ADD_ASSIMP_IMPORTER( IRR
  Irr/IRRLoader.cpp
  Irr/IRRLoader.h
  Irr/IRRShared.cpp
  Irr/IRRShared.h
)

ADD_ASSIMP_IMPORTER( LWO
  LWO/LWOAnimation.cpp
  LWO/LWOAnimation.h
  LWO/LWOBLoader.cpp
  LWO/LWOFileData.h
  LWO/LWOLoader.cpp
  LWO/LWOLoader.h
  LWO/LWOMaterial.cpp
)

ADD_ASSIMP_IMPORTER( LWS
  LWS/LWSLoader.cpp
  LWS/LWSLoader.h
)

ADD_ASSIMP_IMPORTER( MD2
  MD2/MD2FileData.h
  MD2/MD2Loader.cpp
  MD2/MD2Loader.h
  MD2/MD2NormalTable.h
)

ADD_ASSIMP_IMPORTER( MD3
  MD3/MD3FileData.h
  MD3/MD3Loader.cpp
  MD3/MD3Loader.h
)

ADD_ASSIMP_IMPORTER( MD5
  MD5/MD5Loader.cpp
  MD5/MD5Loader.h
  MD5/MD5Parser.cpp
  MD5/MD5Parser.h
)

ADD_ASSIMP_IMPORTER( MDC
  MDC/MDCFileData.h
  MDC/MDCLoader.cpp
  MDC/MDCLoader.h
  MDC/MDCNormalTable.h
)

ADD_ASSIMP_IMPORTER( MDL
  MDL/MDLDefaultColorMap.h
  MDL/MDLFileData.h
  MDL/MDLLoader.cpp
  MDL/MDLLoader.h
  MDL/MDLMaterialLoader.cpp
)

SET( MaterialSystem_SRCS
  Material/MaterialSystem.cpp
  Material/MaterialSystem.h
)

ADD_ASSIMP_IMPORTER( NFF
  NFF/NFFLoader.cpp
  NFF/NFFLoader.h
)

ADD_ASSIMP_IMPORTER( NDO
  NDO/NDOLoader.cpp
  NDO/NDOLoader.h
)

ADD_ASSIMP_IMPORTER( OFF
  OFF/OFFLoader.cpp
  OFF/OFFLoader.h
)

ADD_ASSIMP_IMPORTER( OBJ
  Obj/ObjFileData.h
  Obj/ObjFileImporter.cpp
  Obj/ObjFileImporter.h
  Obj/ObjFileMtlImporter.cpp
  Obj/ObjFileMtlImporter.h
  Obj/ObjFileParser.cpp
  Obj/ObjFileParser.h
  Obj/ObjTools.h
)

ADD_ASSIMP_EXPORTER( OBJ
  Obj/ObjExporter.h
  Obj/ObjExporter.cpp
)

ADD_ASSIMP_IMPORTER( OGRE
  Ogre/OgreImporter.h
  Ogre/OgreStructs.h
  Ogre/OgreParsingUtils.h
  Ogre/OgreBinarySerializer.h
  Ogre/OgreXmlSerializer.h
  Ogre/OgreImporter.cpp
  Ogre/OgreStructs.cpp
  Ogre/OgreBinarySerializer.cpp
  Ogre/OgreXmlSerializer.cpp
  Ogre/OgreMaterial.cpp
)

ADD_ASSIMP_IMPORTER( OPENGEX
  OpenGEX/OpenGEXImporter.cpp
  OpenGEX/OpenGEXImporter.h
  OpenGEX/OpenGEXStructs.h
)

ADD_ASSIMP_EXPORTER( OPENGEX
  OpenGEX/OpenGEXExporter.cpp
  OpenGEX/OpenGEXExporter.h
)

ADD_ASSIMP_IMPORTER( PLY
  Ply/PlyLoader.cpp
  Ply/PlyLoader.h
  Ply/PlyParser.cpp
  Ply/PlyParser.h
)

ADD_ASSIMP_EXPORTER( PLY
  Ply/PlyExporter.cpp
  Ply/PlyExporter.h
)

ADD_ASSIMP_IMPORTER( MS3D
  MS3D/MS3DLoader.cpp
  MS3D/MS3DLoader.h
)

ADD_ASSIMP_IMPORTER( COB
  COB/COBLoader.cpp
  COB/COBLoader.h
  COB/COBScene.h
)

ADD_ASSIMP_IMPORTER( BLEND
  Blender/BlenderLoader.cpp
  Blender/BlenderLoader.h
  Blender/BlenderDNA.cpp
  Blender/BlenderDNA.h
  Blender/BlenderDNA.inl
  Blender/BlenderScene.cpp
  Blender/BlenderScene.h
  Blender/BlenderSceneGen.h
  Blender/BlenderIntermediate.h
  Blender/BlenderModifier.h
  Blender/BlenderModifier.cpp
  Blender/BlenderBMesh.h
  Blender/BlenderBMesh.cpp
  Blender/BlenderTessellator.h
  Blender/BlenderTessellator.cpp
  Blender/BlenderCustomData.h
  Blender/BlenderCustomData.cpp
)

ADD_ASSIMP_IMPORTER( IFC
  Importer/IFC/IFCLoader.cpp
  Importer/IFC/IFCLoader.h
  Importer/IFC/IFCReaderGen1_2x3.cpp
  Importer/IFC/IFCReaderGen2_2x3.cpp
  Importer/IFC/IFCReaderGen_2x3.h
  Importer/IFC/IFCUtil.h
  Importer/IFC/IFCUtil.cpp
  Importer/IFC/IFCGeometry.cpp
  Importer/IFC/IFCMaterial.cpp
  Importer/IFC/IFCProfile.cpp
  Importer/IFC/IFCCurve.cpp
  Importer/IFC/IFCBoolean.cpp
  Importer/IFC/IFCOpenings.cpp
)

if (ASSIMP_BUILD_IFC_IMPORTER)
  if (MSVC)
    set_source_files_properties(Importer/IFC/IFCReaderGen1_2x3.cpp Importer/IFC/IFCReaderGen2_2x3.cpp PROPERTIES COMPILE_FLAGS "/bigobj")
  elseif(CMAKE_COMPILER_IS_MINGW)
    set_source_files_properties(Importer/IFC/IFCReaderGen1_2x3.cpp Importer/IFC/IFCReaderGen2_2x3.cpp PROPERTIES COMPILE_FLAGS "-O2 -Wa,-mbig-obj")
  endif()
endif (ASSIMP_BUILD_IFC_IMPORTER)

ADD_ASSIMP_IMPORTER( XGL
  XGL/XGLLoader.cpp
  XGL/XGLLoader.h
)

ADD_ASSIMP_IMPORTER( FBX
  FBX/FBXImporter.cpp
  FBX/FBXCompileConfig.h
  FBX/FBXImporter.h
  FBX/FBXParser.cpp
  FBX/FBXParser.h
  FBX/FBXTokenizer.cpp
  FBX/FBXTokenizer.h
  FBX/FBXImportSettings.h
  FBX/FBXConverter.h
  FBX/FBXConverter.cpp
  FBX/FBXUtil.h
  FBX/FBXUtil.cpp
  FBX/FBXDocument.h
  FBX/FBXDocument.cpp
  FBX/FBXProperties.h
  FBX/FBXProperties.cpp
  FBX/FBXMeshGeometry.h
  FBX/FBXMeshGeometry.cpp
  FBX/FBXMaterial.cpp
  FBX/FBXModel.cpp
  FBX/FBXAnimation.cpp
  FBX/FBXNodeAttribute.cpp
  FBX/FBXDeformer.cpp
  FBX/FBXBinaryTokenizer.cpp
  FBX/FBXDocumentUtil.cpp
  FBX/FBXCommon.h
)

ADD_ASSIMP_EXPORTER( FBX
  FBX/FBXExporter.h
  FBX/FBXExporter.cpp
  FBX/FBXExportNode.h
  FBX/FBXExportNode.cpp
  FBX/FBXExportProperty.h
  FBX/FBXExportProperty.cpp
)

SET( PostProcessing_SRCS
  PostProcessing/CalcTangentsProcess.cpp
  PostProcessing/CalcTangentsProcess.h
  PostProcessing/ComputeUVMappingProcess.cpp
  PostProcessing/ComputeUVMappingProcess.h
  PostProcessing/ConvertToLHProcess.cpp
  PostProcessing/ConvertToLHProcess.h
  PostProcessing/EmbedTexturesProcess.cpp
  PostProcessing/EmbedTexturesProcess.h
  PostProcessing/FindDegenerates.cpp
  PostProcessing/FindDegenerates.h
  PostProcessing/FindInstancesProcess.cpp
  PostProcessing/FindInstancesProcess.h
  PostProcessing/FindInvalidDataProcess.cpp
  PostProcessing/FindInvalidDataProcess.h
  PostProcessing/FixNormalsStep.cpp
  PostProcessing/FixNormalsStep.h
  PostProcessing/DropFaceNormalsProcess.cpp
  PostProcessing/DropFaceNormalsProcess.h
  PostProcessing/GenFaceNormalsProcess.cpp
  PostProcessing/GenFaceNormalsProcess.h
  PostProcessing/GenVertexNormalsProcess.cpp
  PostProcessing/GenVertexNormalsProcess.h
  PostProcessing/PretransformVertices.cpp
  PostProcessing/PretransformVertices.h
  PostProcessing/ImproveCacheLocality.cpp
  PostProcessing/ImproveCacheLocality.h
  PostProcessing/JoinVerticesProcess.cpp
  PostProcessing/JoinVerticesProcess.h
  PostProcessing/LimitBoneWeightsProcess.cpp
  PostProcessing/LimitBoneWeightsProcess.h
  PostProcessing/RemoveRedundantMaterials.cpp
  PostProcessing/RemoveRedundantMaterials.h
  PostProcessing/RemoveVCProcess.cpp
  PostProcessing/RemoveVCProcess.h
  PostProcessing/SortByPTypeProcess.cpp
  PostProcessing/SortByPTypeProcess.h
  PostProcessing/SplitLargeMeshes.cpp
  PostProcessing/SplitLargeMeshes.h
  PostProcessing/TextureTransform.cpp
  PostProcessing/TextureTransform.h
  PostProcessing/TriangulateProcess.cpp
  PostProcessing/TriangulateProcess.h
  PostProcessing/ValidateDataStructure.cpp
  PostProcessing/ValidateDataStructure.h
  PostProcessing/OptimizeGraph.cpp
  PostProcessing/OptimizeGraph.h
  PostProcessing/OptimizeMeshes.cpp
  PostProcessing/OptimizeMeshes.h
  PostProcessing/DeboneProcess.cpp
  PostProcessing/DeboneProcess.h
  PostProcessing/ProcessHelper.h
  PostProcessing/ProcessHelper.cpp
  PostProcessing/MakeVerboseFormat.cpp
  PostProcessing/MakeVerboseFormat.h
  PostProcessing/ScaleProcess.cpp
  PostProcessing/ScaleProcess.h
  PostProcessing/GenBoundingBoxesProcess.cpp
  PostProcessing/GenBoundingBoxesProcess.h
)

SET( IrrXML_SRCS
  ../contrib/irrXML/irrXML.cpp
  ${HEADER_PATH}/irrXMLWrapper.h
)

ADD_ASSIMP_IMPORTER( Q3D
  Q3D/Q3DLoader.cpp
  Q3D/Q3DLoader.h
)

ADD_ASSIMP_IMPORTER( Q3BSP
  Q3BSP/Q3BSPFileData.h
  Q3BSP/Q3BSPFileParser.h
  Q3BSP/Q3BSPFileParser.cpp
  Q3BSP/Q3BSPFileImporter.h
  Q3BSP/Q3BSPFileImporter.cpp
)

ADD_ASSIMP_IMPORTER( RAW
  Raw/RawLoader.cpp
  Raw/RawLoader.h
)

ADD_ASSIMP_IMPORTER( SIB
  SIB/SIBImporter.cpp
  SIB/SIBImporter.h
)

ADD_ASSIMP_IMPORTER( SMD
  SMD/SMDLoader.cpp
  SMD/SMDLoader.h
)

ADD_ASSIMP_IMPORTER( STL
  STL/STLLoader.cpp
  STL/STLLoader.h
)

ADD_ASSIMP_EXPORTER( STL
  STL/STLExporter.h
  STL/STLExporter.cpp
)

ADD_ASSIMP_IMPORTER( TERRAGEN
  Terragen/TerragenLoader.cpp
  Terragen/TerragenLoader.h
)

ADD_ASSIMP_IMPORTER( 3D
  Unreal/UnrealLoader.cpp
  Unreal/UnrealLoader.h
)

ADD_ASSIMP_IMPORTER( X
  X/XFileHelper.h
  X/XFileImporter.cpp
  X/XFileImporter.h
  X/XFileParser.cpp
  X/XFileParser.h
)

ADD_ASSIMP_EXPORTER( X
  X/XFileExporter.h
  X/XFileExporter.cpp
)

ADD_ASSIMP_IMPORTER( X3D
  X3D/X3DImporter.cpp
  X3D/X3DImporter.hpp
  X3D/X3DImporter_Geometry2D.cpp
  X3D/X3DImporter_Geometry3D.cpp
  X3D/X3DImporter_Group.cpp
  X3D/X3DImporter_Light.cpp
  X3D/X3DImporter_Macro.hpp
  X3D/X3DImporter_Metadata.cpp
  X3D/X3DImporter_Networking.cpp
  X3D/X3DImporter_Node.hpp
  X3D/X3DImporter_Postprocess.cpp
  X3D/X3DImporter_Rendering.cpp
  X3D/X3DImporter_Shape.cpp
  X3D/X3DImporter_Texturing.cpp
  X3D/FIReader.hpp
  X3D/FIReader.cpp
  X3D/X3DVocabulary.cpp
)

ADD_ASSIMP_EXPORTER( X3D
  X3D/X3DExporter.cpp
  X3D/X3DExporter.hpp
)

ADD_ASSIMP_IMPORTER( GLTF
  glTF/glTFCommon.h
  glTF/glTFCommon.cpp
  glTF/glTFAsset.h
  glTF/glTFAsset.inl
  glTF/glTFAssetWriter.h
  glTF/glTFAssetWriter.inl
  glTF/glTFImporter.cpp
  glTF/glTFImporter.h
  glTF2/glTF2Asset.h
  glTF2/glTF2Asset.inl
  glTF2/glTF2AssetWriter.h
  glTF2/glTF2AssetWriter.inl
  glTF2/glTF2Importer.cpp
  glTF2/glTF2Importer.h
)

ADD_ASSIMP_EXPORTER( GLTF
  glTF/glTFExporter.h
  glTF/glTFExporter.cpp
  glTF2/glTF2Exporter.h
  glTF2/glTF2Exporter.cpp
)

ADD_ASSIMP_IMPORTER( 3MF
  3MF/D3MFImporter.h
  3MF/D3MFImporter.cpp
  3MF/D3MFOpcPackage.h
  3MF/D3MFOpcPackage.cpp
  3MF/3MFXmlTags.h
)

ADD_ASSIMP_EXPORTER( 3MF
  3MF/D3MFExporter.h
  3MF/D3MFExporter.cpp
)

ADD_ASSIMP_IMPORTER( MMD
  MMD/MMDCpp14.h
  MMD/MMDImporter.cpp
  MMD/MMDImporter.h
  MMD/MMDPmdParser.h
  MMD/MMDPmxParser.h
  MMD/MMDPmxParser.cpp
  MMD/MMDVmdParser.h
)

ADD_ASSIMP_EXPORTER( ASSJSON
  Assjson/cencode.c
  Assjson/cencode.h
  Assjson/json_exporter.cpp
  Assjson/mesh_splitter.cpp
  Assjson/mesh_splitter.h
)

ADD_ASSIMP_IMPORTER( STEP
    Step/STEPFile.h
    Importer/StepFile/StepFileImporter.h
    Importer/StepFile/StepFileImporter.cpp
    Importer/StepFile/StepFileGen1.cpp
    Importer/StepFile/StepFileGen2.cpp
    Importer/StepFile/StepFileGen3.cpp
    Importer/StepFile/StepReaderGen.h
)

ADD_ASSIMP_EXPORTER( STEP
    Step/StepExporter.h
    Step/StepExporter.cpp
)

if ((NOT ASSIMP_NO_EXPORT) OR (NOT ASSIMP_EXPORTERS_ENABLED STREQUAL ""))
	SET( Exporter_SRCS
	  Common/Exporter.cpp
	  CApi/AssimpCExport.cpp
	  ${HEADER_PATH}/BlobIOSystem.h
	)
endif()

SET( Extra_SRCS
  MD4FileData.h
)

# polyclipping
SET( Clipper_SRCS
  ../contrib/clipper/clipper.hpp
  ../contrib/clipper/clipper.cpp
)

# poly2tri
SET( Poly2Tri_SRCS
  ../contrib/poly2tri/poly2tri/common/shapes.cc
  ../contrib/poly2tri/poly2tri/common/shapes.h
  ../contrib/poly2tri/poly2tri/common/utils.h
  ../contrib/poly2tri/poly2tri/sweep/advancing_front.h
  ../contrib/poly2tri/poly2tri/sweep/advancing_front.cc
  ../contrib/poly2tri/poly2tri/sweep/cdt.cc
  ../contrib/poly2tri/poly2tri/sweep/cdt.h
  ../contrib/poly2tri/poly2tri/sweep/sweep.cc
  ../contrib/poly2tri/poly2tri/sweep/sweep.h
  ../contrib/poly2tri/poly2tri/sweep/sweep_context.cc
  ../contrib/poly2tri/poly2tri/sweep/sweep_context.h
)

# minizip/unzip
SET( unzip_SRCS
  ../contrib/unzip/crypt.h
  ../contrib/unzip/ioapi.c
  ../contrib/unzip/ioapi.h
  ../contrib/unzip/unzip.c
  ../contrib/unzip/unzip.h
)

# zip (https://github.com/kuba--/zip)
SET( ziplib_SRCS
  ../contrib/zip/src/miniz.h
  ../contrib/zip/src/zip.c
  ../contrib/zip/src/zip.h
)

# TODO if cmake required version has been updated to >3.12.0, collapse this to the second case only
if(${CMAKE_VERSION} VERSION_LESS "3.12.0")
	add_definitions(-DMINIZ_USE_UNALIGNED_LOADS_AND_STORES=0)
else()
	add_compile_definitions(MINIZ_USE_UNALIGNED_LOADS_AND_STORES=0)
endif()

# openddlparser
SET ( openddl_parser_SRCS
  ../contrib/openddlparser/code/OpenDDLParser.cpp
  ../contrib/openddlparser/code/DDLNode.cpp
  ../contrib/openddlparser/code/OpenDDLCommon.cpp
  ../contrib/openddlparser/code/OpenDDLExport.cpp
  ../contrib/openddlparser/code/Value.cpp
  ../contrib/openddlparser/code/OpenDDLStream.cpp
  ../contrib/openddlparser/include/openddlparser/OpenDDLParser.h
  ../contrib/openddlparser/include/openddlparser/OpenDDLParserUtils.h
  ../contrib/openddlparser/include/openddlparser/OpenDDLCommon.h
  ../contrib/openddlparser/include/openddlparser/OpenDDLExport.h
  ../contrib/openddlparser/include/openddlparser/OpenDDLStream.h
  ../contrib/openddlparser/include/openddlparser/DDLNode.h
  ../contrib/openddlparser/include/openddlparser/Value.h
)

# RapidJSON
INCLUDE_DIRECTORIES( "../contrib/rapidjson/include" )
INCLUDE_DIRECTORIES( "../contrib" )

SET (unzip_compile_SRCS ${unzip_SRCS})
INCLUDE_DIRECTORIES( "../contrib/unzip/" )

MESSAGE(STATUS "Enabled importer formats:${ASSIMP_IMPORTERS_ENABLED}")
MESSAGE(STATUS "Disabled importer formats:${ASSIMP_IMPORTERS_DISABLED}")

MESSAGE(STATUS "Enabled exporter formats:${ASSIMP_EXPORTERS_ENABLED}")
MESSAGE(STATUS "Disabled exporter formats:${ASSIMP_EXPORTERS_DISABLED}")

SET( assimp_src
  # Assimp Files
  ${Core_SRCS}
  ${CApi_SRCS}
  ${Common_SRCS}
  ${Logging_SRCS}
  ${Exporter_SRCS}
  ${PostProcessing_SRCS}
  ${MaterialSystem_SRCS}
  ${STEPParser_SRCS}
#  ${Step_SRCS} check if we need a different approach

  # Model Support
  ${ASSIMP_LOADER_SRCS}
  ${ASSIMP_EXPORTER_SRCS}

  # Third-party libraries
  ${IrrXML_SRCS}
  ${unzip_compile_SRCS}
  ${Poly2Tri_SRCS}
  ${Clipper_SRCS}
  ${openddl_parser_SRCS}
  ${open3dgc_SRCS}
  ${ziplib_SRCS}
  # Necessary to show the headers in the project when using the VC++ generator:

  ${PUBLIC_HEADERS}
  ${COMPILER_HEADERS}
)
ADD_DEFINITIONS( -DOPENDDLPARSER_BUILD )
ADD_DEFINITIONS( -DASSIMP_BUILD_NO_C4D_IMPORTER )

INCLUDE_DIRECTORIES(
  ../contrib/openddlparser/include
  ../contrib/irrXML
)

include_directories(.)

ADD_LIBRARY(assimp ${assimp_src})

target_link_libraries(assimp ${ZLIB_LIBRARIES})
