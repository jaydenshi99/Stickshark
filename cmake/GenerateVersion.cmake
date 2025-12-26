# Script to generate VERSION.txt with current timestamp
# This runs on every build to ensure timestamp is always current

string(TIMESTAMP BUILD_DATE "%Y-%m-%d %H:%M:%S UTC" UTC)

file(WRITE "${VERSION_FILE}"
    "Stickshark Chess Engine\nVersion ${PROJECT_VERSION}\nBuild Date: ${BUILD_DATE}\n")
