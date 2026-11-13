# -*- coding: utf-8 -*-

"""Download audio from a YouTube URL and convert it to a WAV file."""

import argparse
import importlib.util
import shutil
import subprocess
import sys
from pathlib import Path
from urllib.parse import parse_qs, urlencode, urlparse, urlunparse


def _ensure_binary(binary_name, install_hint):
    if shutil.which(binary_name) is None:
        raise RuntimeError(install_hint)


def _resolve_yt_dlp_launcher(explicit_binary=None):
    if explicit_binary:
        explicit_path = Path(explicit_binary).expanduser().resolve()
        if not explicit_path.exists():
            raise RuntimeError(f"yt-dlp binary not found: {explicit_path}")
        return [str(explicit_path)]

    homebrew_binary = Path("/opt/homebrew/bin/yt-dlp")
    if homebrew_binary.exists():
        return [str(homebrew_binary)]

    yt_dlp_binary = shutil.which("yt-dlp")
    if yt_dlp_binary:
        return [str(Path(yt_dlp_binary).expanduser().resolve())]

    if importlib.util.find_spec("yt_dlp") is not None:
        return [sys.executable, "-m", "yt_dlp"]

    raise RuntimeError(
        "yt-dlp is not installed. Install it with pip in this environment or install the standalone 'yt-dlp' binary."
    )


def _normalise_url(url, allow_playlist):
    cleaned_url = (
        url.strip()
        .replace("\\?", "?")
        .replace("\\&", "&")
        .replace("\\=", "=")
    )

    parsed = urlparse(cleaned_url)
    host = parsed.netloc.lower()

    if allow_playlist:
        return cleaned_url

    if "youtube.com" in host:
        query = parse_qs(parsed.query)
        video_id = query.get("v", [None])[0]
        if video_id:
            canonical_query = {"v": video_id}
            if "t" in query:
                canonical_query["t"] = query["t"][0]
            return urlunparse(
                (
                    parsed.scheme or "https",
                    parsed.netloc,
                    "/watch",
                    "",
                    urlencode(canonical_query),
                    "",
                )
            )

    if "youtu.be" in host:
        video_id = parsed.path.lstrip("/")
        if video_id:
            query = parse_qs(parsed.query)
            canonical_query = {}
            if "t" in query:
                canonical_query["t"] = query["t"][0]
            return urlunparse(
                (
                    parsed.scheme or "https",
                    parsed.netloc,
                    f"/{video_id}",
                    "",
                    urlencode(canonical_query),
                    "",
                )
            )

    return cleaned_url


def _build_command(args, output_template, yt_dlp_launcher):
    command = list(yt_dlp_launcher) + [
        "--extract-audio",
        "--audio-format",
        "wav",
        "--audio-quality",
        "0",
        "--format",
        args.format,
        "--output",
        str(output_template),
        "--postprocessor-args",
        f"ffmpeg:-ar {args.sample_rate} -ac {1 if args.mono else 2}",
    ]

    if not args.playlist:
        command.append("--no-playlist")

    if args.cookies:
        command.extend(["--cookies", args.cookies])

    if args.cookies_from_browser:
        command.extend(["--cookies-from-browser", args.cookies_from_browser])

    command.append(_normalise_url(args.url, args.playlist))
    return command


def main():
    parser = argparse.ArgumentParser(
        description=(
            "Download audio from a YouTube URL and convert it to a WAV file. "
            "Use only with audio you have permission to download."
        )
    )
    parser.add_argument("url", help="YouTube video or playlist URL")
    parser.add_argument(
        "-o",
        "--output-dir",
        default=".",
        help="Directory to write the downloaded audio into"
    )
    parser.add_argument(
        "-n",
        "--output-name",
        default=None,
        help="Base filename to use instead of the video title"
    )
    parser.add_argument(
        "--sample-rate",
        type=int,
        default=22050,
        help="Output WAV sample rate in Hz"
    )
    parser.add_argument(
        "--mono",
        action="store_true",
        help="Convert the output WAV to mono"
    )
    parser.add_argument(
        "--playlist",
        action="store_true",
        help="Allow playlist downloads instead of forcing a single video"
    )
    parser.add_argument(
        "--format",
        default="bestaudio/best",
        help="yt-dlp format selector for the source stream"
    )
    parser.add_argument(
        "--cookies",
        default=None,
        help="Optional cookies.txt file for age-gated or authenticated videos"
    )
    parser.add_argument(
        "--cookies-from-browser",
        default=None,
        help="Browser name for yt-dlp cookie extraction, for example chrome, safari, or firefox"
    )
    parser.add_argument(
        "--yt-dlp-bin",
        default=None,
        help="Explicit path to the yt-dlp binary to use"
    )
    args = parser.parse_args()

    if args.sample_rate <= 0:
        parser.error("--sample-rate must be greater than zero")

    _ensure_binary(
        "ffmpeg",
        "ffmpeg is not installed or not on PATH. Install ffmpeg before running this script."
    )
    yt_dlp_launcher = _resolve_yt_dlp_launcher(args.yt_dlp_bin)

    output_dir = Path(args.output_dir).expanduser().resolve()
    output_dir.mkdir(parents=True, exist_ok=True)

    if args.output_name:
        output_template = output_dir / f"{args.output_name}.%(ext)s"
    else:
        output_template = output_dir / "%(title)s [%(id)s].%(ext)s"

    if args.cookies and args.cookies_from_browser:
        parser.error("use either --cookies or --cookies-from-browser, not both")

    if args.cookies:
        cookies_path = Path(args.cookies).expanduser().resolve()
        if not cookies_path.exists():
            parser.error(f"--cookies file does not exist: {cookies_path}")
        args.cookies = str(cookies_path)

    command = _build_command(args, output_template, yt_dlp_launcher)
    subprocess.run(command, check=True)

    print(f"Downloaded audio into {output_dir}")


if __name__ == "__main__":
    main()
