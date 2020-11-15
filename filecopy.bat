rem https://www.sourcemodding.com/tutorials/goldsrc/projects/where-is-poppy/setting-up-a-goldsrc-mod/#Modify_the_Post_Build_Event
@echo on

set "sourcePath=%~f1"
set "targetPath=%~f2"

rem Prevents the compile from failing if the target path doesn't exist
rem Brushtool did not account for this, it seems
if not exist "%targetPath%" (
	echo "filecopy :: targetPath does not exist, you'll need to copy binaries manually"
	exit 0
)

echo "filecopy :: SourcePath : %sourcePath%"
echo "filecopy :: TargetPath : %targetPath%"

@copy "%sourcePath%" "%targetPath%"
