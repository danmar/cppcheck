import unittest
from unittest.mock import patch, MagicMock, call
import sys
import os
import tempfile
import shutil
import json
import io
from contextlib import redirect_stdout
from pathlib import Path

# Add addons and tools to sys.path to allow for imports
# Assuming the script is run from the root of the cppcheck repository
sys.path.insert(0, str(Path(__file__).parent.parent.parent))
from addons import y2038_buildsystem

class TestY2038Buildsystem(unittest.TestCase):

    def setUp(self):
        self.test_dir = tempfile.mkdtemp()

    def tearDown(self):
        shutil.rmtree(self.test_dir)

    def _create_dummy_project(self, build_system_file):
        project_dir = Path(self.test_dir)
        (project_dir / build_system_file).touch()
        return project_dir

    def _create_dummy_compile_commands(self, directory):
        content = [
            {
                "directory": str(directory),
                "command": "gcc -o test.o -c src/test.c",
                "file": "src/test.c"
            }
        ]
        compile_commands_path = directory / "compile_commands.json"
        with open(compile_commands_path, 'w') as f:
            json.dump(content, f)
        return compile_commands_path

    @patch('addons.y2038_buildsystem.run_command')
    def test_cmake_project(self, mock_run_command):
        """Test detection and handling of a CMake project."""
        project_dir = self._create_dummy_project("CMakeLists.txt")
        build_dir = project_dir / "build"

        def mock_cmake_run(*args, **kwargs):
            self._create_dummy_compile_commands(build_dir)
            # After creating the dummy file, we need to copy it to the project root
            shutil.copy(build_dir / "compile_commands.json", project_dir)
            return True

        mock_run_command.side_effect = mock_cmake_run

        # Test the library function directly
        result = y2038_buildsystem.generate_compile_commands(str(project_dir))
        
        # Check that the build command was called
        mock_run_command.assert_called_once_with(
            ["cmake", "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON", ".."],
            cwd=build_dir
        )
        
        # Verify the function returned success
        self.assertTrue(result)


    @patch('addons.y2038_buildsystem.run_command')
    @patch('shutil.which', return_value='/usr/bin/bear')
    def test_make_project(self, mock_shutil_which, mock_run_command):
        """Test detection and handling of a Make project."""
        project_dir = self._create_dummy_project("Makefile")

        def mock_bear_run(*args, **kwargs):
            self._create_dummy_compile_commands(project_dir)
            return True

        mock_run_command.side_effect = mock_bear_run

        # Test the library function directly
        result = y2038_buildsystem.generate_compile_commands(str(project_dir))

        mock_run_command.assert_called_once_with(
            ["bear", "--", "make"],
            cwd=project_dir
        )
        
        # Verify the function returned success
        self.assertTrue(result)


    @patch('addons.y2038_buildsystem.run_command')
    @patch('shutil.which', return_value='/usr/bin/bear')
    def test_autotools_project(self, mock_shutil_which, mock_run_command):
        """Test detection and handling of an Autotools project."""
        project_dir = self._create_dummy_project("configure")
        (project_dir / "Makefile").touch()

        def mock_autotools_run(cmd, cwd):
            if cmd[0] == "./configure":
                return True
            if cmd[0] == "bear":
                self._create_dummy_compile_commands(project_dir)
                return True
            return False

        mock_run_command.side_effect = mock_autotools_run

        # Test the library function directly
        result = y2038_buildsystem.generate_compile_commands(str(project_dir))

        expected_calls = [
            call(["./configure"], cwd=project_dir),
            call(["bear", "--", "make"], cwd=project_dir)
        ]
        mock_run_command.assert_has_calls(expected_calls)
        
        # Verify the function returned success
        self.assertTrue(result)

    @patch('addons.y2038_buildsystem.detect_build_system', return_value=None)
    def test_no_build_system_detected(self, mock_detect):
        """Test that the script exits gracefully if no build system is detected."""
        project_dir = Path(self.test_dir)
        with redirect_stdout(io.StringIO()):
            with patch.object(sys, 'argv', ['y2038_buildsystem.py', str(project_dir)]):
                with self.assertRaises(SystemExit) as cm:
                    y2038_buildsystem.main()
                self.assertEqual(cm.exception.code, 1)

    @patch('addons.y2038_buildsystem.generate_for_cmake', return_value=False)
    @patch('addons.y2038_buildsystem.detect_build_system', return_value='cmake')
    def test_generation_failure(self, mock_detect, mock_generate):
        """Test that the script exits gracefully if compile_commands.json generation fails."""
        project_dir = Path(self.test_dir)
        with redirect_stdout(io.StringIO()):
            with patch.object(sys, 'argv', ['y2038_buildsystem.py', str(project_dir)]):
                with self.assertRaises(SystemExit) as cm:
                    y2038_buildsystem.main()
                self.assertEqual(cm.exception.code, 1)

    def test_library_usage(self):
        """Test that y2038_buildsystem can be used as a library"""
        project_dir = self._create_dummy_project("CMakeLists.txt")
        
        # Test generate_compile_commands function
        result = y2038_buildsystem.generate_compile_commands(str(project_dir))
        # The function should return False since we don't have actual CMake installed
        # but it should not raise an exception
        self.assertIsInstance(result, bool)
        
        # Test detect_build_system function
        build_system = y2038_buildsystem.detect_build_system(str(project_dir))
        self.assertEqual(build_system, "cmake")
        
        # Test with no build system
        empty_project = Path(self.test_dir) / "empty"
        empty_project.mkdir()
        build_system = y2038_buildsystem.detect_build_system(str(empty_project))
        self.assertIsNone(build_system)


if __name__ == '__main__':
    unittest.main(verbosity=2)