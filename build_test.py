import subprocess
import shlex

cmd = r'C:\Qt\Tools\mingw1310_64\bin\g++.exe -DHAS_QT_CHARTS -DHAS_QT_DATAVIS -DMINGW_HAS_SECURE_API=1 -DQT_CHARTS_LIB -DQT_CORE_LIB -DQT_DATAVISUALIZATION_LIB -DQT_GUI_LIB -DQT_NETWORK_LIB -DQT_OPENGLWIDGETS_LIB -DQT_OPENGL_LIB -DQT_QML_DEBUG -DQT_SERIALPORT_LIB -DQT_WIDGETS_LIB -DQT_XML_LIB -DUNICODE -DWIN32 -DWIN64 -D_ENABLE_EXTENDED_ALIGNED_STORAGE -D_UNICODE -D_WIN64 @CMakeFiles/OSTuner.dir/includes_CXX.rsp -g -std=gnu++17 -MD -MT CMakeFiles/OSTuner.dir/src/core/ECUDefinition.cpp.obj -MF CMakeFiles\OSTuner.dir\src\core\ECUDefinition.cpp.obj.d -o CMakeFiles\OSTuner.dir\src\core\ECUDefinition.cpp.obj -c "C:\OS Tuner\src\core\ECUDefinition.cpp"'

args = shlex.split(cmd, posix=False)
args[-1] = args[-1].strip('"')

with open("gpp_out.txt", "w") as out, open("gpp_err.txt", "w") as err:
    r = subprocess.run(args, cwd=r'C:\OS Tuner\build\Desktop_Qt_6_10_2_MinGW_64_bit-Debug', stdout=out, stderr=err)
print('CODE:', r.returncode)
