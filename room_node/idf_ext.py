"""Project-level idf.py extension for the two-room room_node build.

Running ``idf.py build`` without a room selection builds both rooms into
``build/LLT_1A`` and ``build/LLT_1B``. Passing ``-DCOCIS_ROOM_ID=1`` (or
``-DCOCIS_ROOM_ID=2``) builds only that room.
"""

import os
import subprocess
import sys

ROOM_BUILDS = (
    ("1", "LLT_1A"),
    ("2", "LLT_1B"),
)


def action_extensions(base_actions, project_path):
    def dispatch_room_build(ctx, global_args, tasks):
        task_names = {task.name for task in tasks}
        if "build" not in task_names and "all" not in task_names:
            return

        define_entries = list(getattr(global_args, "define_cache_entry", []) or [])
        room_id_entries = [
            entry.split("=", 1)[-1].strip()
            for entry in define_entries
            if entry.split("=", 1)[0] == "COCIS_ROOM_ID"
        ]
        if room_id_entries:
            if len(room_id_entries) != 1 or room_id_entries[0] not in dict(ROOM_BUILDS):
                raise RuntimeError(
                    "COCIS_ROOM_ID must be specified exactly once as 1 (LLT_1A) or 2 (LLT_1B)."
                )
            return

        idf_py = os.path.join(os.environ["IDF_PATH"], "tools", "idf.py")
        for room_id, room_name in ROOM_BUILDS:
            command = [
                sys.executable,
                idf_py,
                "-B",
                os.path.join("build", room_name),
                f"-DCOCIS_ROOM_ID={room_id}",
                "build",
            ]
            print(f"Building room {room_name} ({room_id}) in build/{room_name}...")
            result = subprocess.run(command, cwd=project_path)
            if result.returncode != 0:
                ctx.exit(result.returncode)

        ctx.exit(0)

    return {
        "version": "1",
        "global_action_callbacks": [dispatch_room_build],
    }
