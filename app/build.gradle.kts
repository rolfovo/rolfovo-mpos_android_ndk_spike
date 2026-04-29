plugins {
    id("com.android.application")
}

android {
    namespace = "com.example.mposandroid"
    compileSdk = 35

    defaultConfig {
        applicationId = "com.example.mposandroid"
        minSdk = 23
        targetSdk = 35
        versionCode = 1
        versionName = "0.1.0"

        externalNativeBuild {
            cmake {
                cppFlags += listOf("-std=c++17")
                arguments += listOf("-DANDROID_STL=c++_shared")
            }
        }
    }

    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
            version = "3.22.1"
        }
    }
}
