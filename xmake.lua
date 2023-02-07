add_requires("tbox v1.7.3")
add_rules("mode.debug", "mode.release")

target("f5")
    set_kind("binary")
    set_policy("build.across_targets_in_parallel", false)
    add_deps("packcc")
    add_rules("peg")
    set_languages("gnu99")
    add_packages("tbox")
    -- set_default(true)
    add_includedirs("src")
    add_files("src/parser.peg")
    add_files("src/main.c")
	if is_mode("debug") then
        set_optimize("none")
		add_defines("__tb_debug__")
    else
        set_optimize("faster")
	end

target("packcc")
	set_kind("binary")
    set_languages("gnu99")
	add_files("external/packcc/src/packcc.c")

rule("peg")
    set_extensions(".peg")
    on_buildcmd_file(function (target, batchcmds, source, opts)
        local packcc = target:dep("packcc"):targetfile()
        print("EXPECTED PACKCC:", packcc)
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
