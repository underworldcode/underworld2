import os, re, glob
import h5py

def find_swarm_files(folder, tracers_name):
    res = [f for f in os.listdir(folder) if re.search(r'^'+tracers_name+'(-\d.*.h5)', f)]
    res.sort()
    indices = [int(re.search(r'^'+tracers_name+'-(.+?)(.h5)$', f).group(1)) for f in res]
    return res, indices

def find_tracked_fields(folder, tracers_name):
    """ Returns a list of field tracked by the tracers"""
    files = glob.glob(os.path.join(folder, tracers_name)+"*")
    files = [os.path.split(file)[-1][len(tracers_name):] for file in files if "h5" in file]
    out = []
    for file in files:
        match = re.search("^_(.+?)(-[0-9].h5)$", file)
        if match:
            out.append(match.group(1))
    return list(set(out))

def extract_global_indices(output_folder, tracers_name, index):
    global_index_file = """{0}_global_index-{1}.h5""".format(tracers_name, index)
    with h5py.File(os.path.join(output_folder, global_index_file), "r") as h5f:
        global_indices = h5f["data"].value
    return global_indices.ravel()

def extract_tracers_data(output_folder, tracers_name, csv=True):
    import pandas as pd
    swarm_files, indices = find_swarm_files(output_folder, tracers_name)
    if not swarm_files:
        raise ValueError("""Cannot find tracers {0}""".format(tracers_name))

    tracked_fields = find_tracked_fields(output_folder, tracers_name)
    global_indices = extract_global_indices(output_folder, tracers_name, 0)

    id_dict = {}
    id_ = 0
    for index in global_indices:
        id_dict[index] = id_
        id_ += 1

    default_columns = ["Coord1", "Coord2", "Coord3", "FileIndex", "TracerId"]
    df = pd.DataFrame(columns=default_columns + tracked_fields)
    for index in indices:
        dftempo = pd.DataFrame(columns=default_columns + tracked_fields)
        # Extract Coordinates
        file = """{0}-{1}.h5""".format(tracers_name, index)
        file = os.path.join(output_folder, file)
        with h5py.File(file, "r") as h5f:
            dftempo["Coord1"] = h5f["data"].value[:,0]
            dftempo["Coord2"] = h5f["data"].value[:,1]
            try:
                dftempo["Coord3"] = h5f["data"].value[:,2]
            except:
                dftempo["Coord3"] = None
            dftempo["FileIndex"] = index
            dftempo["Time"] = h5f.attrs["time"]

        for field in tracked_fields:
            file = """{0}_{1}-{2}.h5""".format(tracers_name, field, index)
            file = os.path.join(output_folder, file)
            with h5py.File(file, "r") as h5f:
                dftempo[field] = h5f["data"].value.ravel()

        dftempo["TracerId"] = dftempo["global_index"].map(id_dict)
        df = pd.concat([df, dftempo], ignore_index=True, sort=False)

    if csv:
        df.to_csv(tracers_name+".csv")

    return df
