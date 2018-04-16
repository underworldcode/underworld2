#ifndef Types__
#define Types__

/* Data types for geometry blocks */
typedef enum
{
  lucMinType,
  lucLabelType = lucMinType,
  lucPointType,
  lucGridType,
  lucTriangleType,
  lucVectorType,
  lucTracerType,
  lucLineType,
  lucShapeType,
  lucVolumeType,
  lucMaxType
} lucGeometryType;

/* Data types for values to map to visual properties of geometry */
typedef enum
{
  lucMinDataType,
  lucVertexData = lucMinDataType,
  lucNormalData,
  lucVectorData,
  lucColourValueData,   //Legacy
  lucOpacityValueData,  //Legacy
  lucRedValueData,      //Legacy
  lucGreenValueData,    //Legacy
  lucBlueValueData,     //Legacy
  lucIndexData,
  lucXWidthData,        //Legacy
  lucYHeightData,       //Legacy
  lucZLengthData,       //Legacy
  lucRGBAData,
  lucTexCoordData,
  lucSizeData,          //Legacy
  lucLuminanceData,
  lucRGBData,
  lucMaxDataType
} lucGeometryDataType;

#endif /* Types__ */
