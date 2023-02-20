add_requires("tbox v1.7.3", {debug=true})
add_rules("mode.debug", "mode.release")

target("a")
    set_kind("binary")
    set_default(true)
	add_packages("tbox")
	if is_mode("debug") then
        set_optimize("none")
    else
        set_optimize("faster")
	end
    set_policy("build.across_targets_in_parallel", false)
    set_languages("gnu99")
    add_deps("packcc")
    add_rules("peg")
    add_includedirs("src")
    add_files("src/parser.peg")
    add_files("src/main.c")

target("packcc")
	set_kind("binary")
    set_default(false)
    set_languages("gnu99")
	add_files("external/packcc/src/packcc.c")

rule("peg")
    set_extensions(".peg")
    before_buildcmd_file(function (target, batchcmds, source, opts)
        local packcc = target:dep("packcc"):targetfile()
        local base = path.basename(source)
        local c = target:autogenfile(base .. ".c")
        local h = target:autogenfile(base .. ".h")
        local o = target:objectfile(c)
        batchcmds:vrunv(packcc, {"-o", base, source})
        batchcmds:mv(base .. ".c", c)
        batchcmds:mv(base .. ".h", h)
        batchcmds:compile(c, o, {configs={languages="gnu99"}})
        target:add("files", c, {public=true})
        target:add("includedirs", path.directory(h), {public=true})
    end)
