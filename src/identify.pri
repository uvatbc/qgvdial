# For the beta version of the SDK, this part will identify Meego/Harmattan
exists($$QMAKE_INCDIR_QT"/../qmsystem2/qmkeys.h"):!contains(MEEGO_EDITION,harmattan): {
  MEEGO_VERSION_MAJOR     = 1
  MEEGO_VERSION_MINOR     = 2
  MEEGO_VERSION_PATCH     = 0
  MEEGO_EDITION           = harmattan
}

contains(MEEGO_EDITION,harmattan) {
  message(Meego!)
  DEFINES += MEEGO_HARMATTAN
}

symbian {
    exists(isS1) {
        DEFINES += IS_S1
    }
    exists(isS3) {
        DEFINES += IS_S3
    }
    exists(isS3Belle) {
        DEFINES += IS_S3_BELLE
    }
    !exists(isS1) : !exists(isS3) : !exists(isS3Belle) {
        DEFINES += INVALID_TARGET
    }
}

