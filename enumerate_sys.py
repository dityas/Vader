from pathlib import Path


def list_files_recursive(directory):
    
    files = []
    
    for path in Path(directory).rglob("*"):
        if path.is_file():
            files.append(path)

    return files


def get_data(filename: str) -> str:

    try:
        data = ""

        with open(filename) as f:
            data = f.read()

        return data

    except Exception:
        print(f"Could not read {filename}")
        return ""


if __name__ == "__main__":
    directory = "/sys/devices/virtual/dmi/"

    data = dict()
    files = list_files_recursive(directory)
    
    for file in files:
        
        file_data = get_data(str(file))
        data[str(file)] = file_data

    for k in data.keys():
        print(f"{k} -> {data[k].strip()}")
