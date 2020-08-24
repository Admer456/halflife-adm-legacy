rem https://www.sourcemodding.com/tutorials/goldsrc/projects/where-is-poppy/setting-up-a-goldsrc-mod/#Modify_the_Post_Build_Event
@echo on
 
set "sourcePath=%~f1"
set "targetPath=%~f2"
 
echo "SourcePath : %sourcePath%"
echo "TargetPath : %targetPath%"
 
p4 edit "%targetPath%"
p4 add "%targetPath%"
 
@copy "%sourcePath%" "%targetPath%"
