from SCons.Script import Import

from pathlib import Path
import subprocess

Import("env")

ProjectName = "open-airsoft-countdown"
OutputFileName = ProjectName + ".factory.bin"


def merge_firmware(target, source, env):
	project_dir = Path(env.subst("$PROJECT_DIR"))
	build_dir = Path(env.subst("$BUILD_DIR"))

	output_dir = project_dir / "docs" / "firmware"
	output_dir.mkdir(parents=True, exist_ok=True)

	output_file = output_dir / OutputFileName

	platform = env.PioPlatform()
	python_exe = env.subst("$PYTHONEXE")

	esptool_dir = Path(platform.get_package_dir("tool-esptoolpy"))
	esptool_py = esptool_dir / "esptool.py"

	framework_dir = Path(platform.get_package_dir("framework-arduinoespressif32"))
	boot_app0 = framework_dir / "tools" / "partitions" / "boot_app0.bin"

	bootloader = build_dir / "bootloader.bin"
	partitions = build_dir / "partitions.bin"
	firmware = build_dir / "firmware.bin"

	required_files = [
		bootloader,
		partitions,
		boot_app0,
		firmware,
	]

	for file_path in required_files:
		if not file_path.exists():
			raise FileNotFoundError(str(file_path))

	command = [
		python_exe,
		str(esptool_py),
		"--chip",
		"esp32s3",
		"merge_bin",
		"-o",
		str(output_file),
		"0x0000",
		str(bootloader),
		"0x8000",
		str(partitions),
		"0xe000",
		str(boot_app0),
		"0x10000",
		str(firmware),
	]

	print()
	print("Generating ESP Web Tools factory firmware:")
	print(output_file)
	print()

	subprocess.check_call(command)

	print()
	print("Factory firmware generated successfully.")
	print(output_file)
	print()


env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", merge_firmware)