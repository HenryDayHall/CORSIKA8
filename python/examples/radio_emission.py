#!/usr/bin/python3

import argparse
import os

import matplotlib.pyplot as plt
import numpy as np

import corsika

here = os.path.abspath(os.path.dirname(__file__))

parser = argparse.ArgumentParser()
parser.add_argument(
    "--input-dir", required=True, help="output directory of the CORSIKA 8 simulation"
)
parser.add_argument(
    "--output-dir",
    default=os.path.join(here, "example_plots"),
    help="output directory for plots",
)
args = parser.parse_args()

if not os.path.isdir(args.output_dir):
    print("Making directory", args.output_dir)
    os.makedirs(args.output_dir)

# Load the shower simulation output
lib = corsika.Library(args.input_dir)

# Load the primary particle information from the shower
primaries = lib.get("primary").data
n_showers = len(primaries)
primary_config = lib.get("primary").config

found_zhs = False
try:
    waveforms_config_zhs = lib.get("ZHS").config  # meta information
    waveforms_zhs = lib.get("ZHS").astype("pandas")
    antennas_zhs = lib.get("ZHS").get_antennas()
    found_zhs = True
except Exception:
    print("No ZHS waveforms in this directory")


found_coreas = False
try:
    waveforms_config_coreas = lib.get("CoREAS").config  # meta information
    waveforms_coreas = lib.get("CoREAS").astype("pandas")
    antennas_coreas = lib.get("CoREAS").get_antennas()
    found_coreas = True
except Exception:
    print("No CoREAS waveforms in this directory")

if not found_zhs and not found_coreas:
    print("No electric field waveforms are in this file, quitting...")
    exit()

for ishower in range(n_showers):

    primary = primaries[ishower]

    title = f"Primary: {primary.name}," + r" E$_{\rm tot}$:"
    title += f" {primary.total_energy:.2e} {primary_config['units']['energy']}"

    # Get all of the unique antenna names to match up the CoREAS/ZHS waveforms
    ant_names = []
    if found_zhs:
        ant_names += list(waveforms_zhs[str(ishower)].keys())
        zhs_dict = waveforms_zhs[str(ishower)]
    if found_coreas:
        ant_names += list(waveforms_coreas[str(ishower)].keys())
        coreas_dict = waveforms_coreas[str(ishower)]
    ant_names = np.unique(ant_names)

    ncols = 1
    nrows = len(ant_names)
    fig, ax = plt.subplots(
        ncols=ncols,
        nrows=nrows,
        figsize=(ncols * 8, nrows * 3),
        gridspec_kw={"wspace": 0.2, "hspace": 0.3},
    )

    # Make a plot for each of the antenna locations
    for iant, ant_name in enumerate(ant_names):

        # Add blank entry to show colors in legend
        ax[iant].fill_between([], [], [], color="k", label="Ex")
        ax[iant].fill_between([], [], [], color="r", label="Ey")
        ax[iant].fill_between([], [], [], color="b", label="Ez")

        if not iant:
            ax[iant].set_title(title)

        # Make the plots for the ZHS waveforms
        if found_zhs and ant_name in antennas_zhs.keys():
            ant_position = np.array(
                [
                    antennas_zhs[ant_name]["x"],
                    antennas_zhs[ant_name]["y"],
                    antennas_zhs[ant_name]["z"],
                ]
            ).flatten()
            times = np.array(zhs_dict[ant_name]["time"])

            ax[iant].plot(
                times, zhs_dict[ant_name]["Ex"], color="k", linestyle="-", label="ZHS"
            )
            ax[iant].plot(times, zhs_dict[ant_name]["Ey"], color="r", linestyle="-")
            ax[iant].plot(times, zhs_dict[ant_name]["Ez"], color="b", linestyle="-")
            ax[iant].set_xlabel(f"Time [{waveforms_config_zhs['units']['time']}]")
            ax[iant].set_ylabel(
                f"Amp ({waveforms_config_zhs['units']['electric field']})"
            )

        # Make the plots for the CoREAS waveforms
        if found_coreas and ant_name in antennas_coreas.keys():
            ant_position = np.array(
                [
                    antennas_coreas[ant_name]["x"],
                    antennas_coreas[ant_name]["y"],
                    antennas_coreas[ant_name]["z"],
                ]
            ).flatten()
            times = np.array(coreas_dict[ant_name]["time"])

            ax[iant].plot(
                times,
                coreas_dict[ant_name]["Ex"],
                color="k",
                linestyle="--",
                label="CoREAS",
            )
            ax[iant].plot(times, coreas_dict[ant_name]["Ey"], color="r", linestyle="--")
            ax[iant].plot(times, coreas_dict[ant_name]["Ez"], color="b", linestyle="--")
            ax[iant].set_xlabel(f"Time ({waveforms_config_coreas['units']['time']})")
            ax[iant].set_ylabel(
                f"Amp ({waveforms_config_coreas['units']['electric field']})"
            )

        ymin, ymax = ax[iant].get_ylim()
        max_amp = max(abs(ymin), abs(ymax))
        ax[iant].set_ylim(-max_amp, max_amp)
        text = f"Antenna @ ({ant_position[0]:0.0f}, {ant_position[1]:0.0f},"
        text += f" {ant_position[2]:0.0f})"
        ax[iant].text(times[-1], 2 * max_amp * 0.05 - max_amp, text, ha="right")
        ax[iant].legend(loc="upper right")

    plot_path = os.path.join(args.output_dir, f"AntennaWaveforms_Sh{ishower}.png")
    print("Saving", plot_path)
    fig.savefig(plot_path, bbox_inches="tight")
