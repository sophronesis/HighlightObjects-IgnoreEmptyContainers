-- include subprojects
includes("lib/commonlibsse-ng")

-- set project constants
set_project("HighlightObjects")
set_version("1.8.1")
set_license("GPL-3.0")
set_languages("c++23")
set_warnings("allextra")

-- add common rules
add_rules("mode.debug", "mode.releasedbg")

-- add packages
add_requires("simpleini v4.25")

-- define targets
target("HighlightObjects")
    add_rules("commonlibsse-ng.plugin", {
        name = "HighlightObjects",
        author = "godfreysnow",
        description = "Highlights interactable objects under the crosshair or in a radius"
    })

    add_packages("simpleini")

    -- add src files
    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    add_includedirs("src")
    set_pcxxheader("src/pch.h")
