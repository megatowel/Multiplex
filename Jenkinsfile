#!groovy

/**
This is a .groovy script for a parametrized Jenkins Pipeline Job that builds a CMake project.
The job expects a jenkins node with the name 'master'. The repository will be checked out on the master
and then copied to a slave-node that has the tag BuildSlaveTag. Then the job will call
$ cmake -H. -B_build <AdditionalGenerateArguments>
$ cmake --build _build <AdditionalBuildArguments>
    
in order to build the project. The job must provide the following paramters
RepositoryUrl                   - The url of the git repository the contains the projects CMakeLists.txt file in the root directory.
CheckoutDirectory               - A workspace directory on the master and build-slave into which the code is checked-out and which is used for the build.
BuildSlaveTag                   - The tag for the build-slave on which the project is build.
AdditionalGenerateArguments     - Arguments for the cmake generate call.
AdditionalBuildArguments        - Arguments for the cmake build call.   
*/


stage('Build')
{
    node(params.BuildSlaveTag)
    {
        // Build for native platform
        ws(params.CheckoutDirectory)   
        {   
            // debug info
            printJobParameter()
			
			checkout scm
			
            // run cmake generate and build
            runCommand( 'cmake -E remove_directory _build' )                             // make sure the build is clean
            runCommand( 'cmake -H. -B_build --target MTMultiplex' )
            runCommand( 'cmake --build _build' )
            
            echo '----- CMake project was built successfully -----'

            // run cmake generate and cross compile
            runCommand( 'cmake -E remove_directory _build_win' )                             // make sure the build is clean
            runCommand( 'cmake -H. -B_build_win --target MTMultiplex -DCMAKE_TOOLCHAIN_FILE=./cmake/Toolchain-cross-mingw32-linux.cmake' )
            runCommand( 'cmake --build _build_win' )
            
            echo '----- CMake project was built successfully for Win64 -----'

            zip zipFile: 'build-linux64.zip', archive: false, dir: '_build'
            zip zipFile: 'build-win64.zip', archive: false, dir: '_build_win'

            archiveArtifacts artifacts: 'build-linux64.zip', 'build-win64.zip', fingerprint: true
        }
    }
}


def printJobParameter()
{
    def introduction = """
----- Build CMake project -----
RepositoryUrl = ${params.RepositoryUrl}
CheckoutDirectory = ${params.CheckoutDirectory}
BuildSlaveTag = ${params.BuildSlaveTag}
AdditionalGenerateArguments = ${params.AdditionalGenerateArguments}
AdditionalBuildArguments = ${params.AdditionalBuildArguments}
-------------------------------
"""
    
    echo introduction
}

def runCommand( command )
{
    if(isUnix())
    {
        sh command
    }
    else
    {
        bat command
    }
}