if (NOT EXISTS "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/build/cpp-sc2/contrib/SDL-mirror/install_manifest.txt")
    message(FATAL_ERROR "Cannot find install manifest: \"C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/build/cpp-sc2/contrib/SDL-mirror/install_manifest.txt\"")
endif(NOT EXISTS "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/build/cpp-sc2/contrib/SDL-mirror/install_manifest.txt")

file(READ "C:/Users/a/Documents/cmput350-project/2Bs-Bombastic-Botification/build/cpp-sc2/contrib/SDL-mirror/install_manifest.txt" files)
string(REGEX REPLACE "\n" ";" files "${files}")
foreach (file ${files})
    message(STATUS "Uninstalling \"$ENV{DESTDIR}${file}\"")
    execute_process(
        COMMAND C:/cmake-3.22.0-rc2-windows-x86_64/bin/cmake.exe -E remove "$ENV{DESTDIR}${file}"
        OUTPUT_VARIABLE rm_out
        RESULT_VARIABLE rm_retval
    )
    if(NOT ${rm_retval} EQUAL 0)
        message(FATAL_ERROR "Problem when removing \"$ENV{DESTDIR}${file}\"")
    endif (NOT ${rm_retval} EQUAL 0)
endforeach(file)

