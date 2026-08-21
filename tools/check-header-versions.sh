#!/usr/bin/env bash
#
# A released version number must never name two different files.
#
# Each header carries its own semantic version in a banner comment on the
# first line after the licence. That number is what a consumer reads to tell
# one vendored copy from another, so it has to change whenever the code
# changes. Twice in two days it did not: the released asset and main both
# called themselves v0.2.0, and later both called themselves v0.2.1, while
# computing different times. Nothing caught either case except a person
# noticing.
#
# The check compares each header against every published release asset. If a
# release already carries this version string and its code differs from the
# working copy, the version is stale and must be bumped. Comments are stripped
# before comparing, so prose edits, new documentation and typo fixes do not
# demand a version bump; only code does.
#
# Passing means the version in the tree names exactly one file, or names a
# file no release has published yet. It does not mean the version was bumped
# in this pull request, and it should not: several pull requests can land
# between releases and only the first needs to move the number.

set -uo pipefail

HEADERS=(prayertimes.h hijri.h timezone.h)

repo_root=$(git rev-parse --show-toplevel)
cd "$repo_root" || exit 1

work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT

# Reduce a header to just its code. -fpreprocessed stops the preprocessor from
# expanding includes or macros, -dD keeps the directives, and comments are
# dropped either way. The "# 149 file" line markers move whenever a comment
# above them changes, so they are filtered out too.
strip_comments() {
    gcc -fpreprocessed -dD -E "$1" 2>/dev/null | grep -v '^# [0-9]'
}

# The banner is the first "<name>.h -- vX.Y.Z --" line in the file. Two comment
# styles are in use, prayertimes.h opens the comment on the same line as the
# banner and timezone.h puts it on the line below, so the opener is ignored
# and only the banner text itself is matched.
header_version() {
    grep -m1 -oE '[a-z_]+\.h -- v[0-9]+(\.[0-9]+)* --' "$1" |
        grep -oE 'v[0-9]+(\.[0-9]+)*'
}

releases=$(gh release list --limit 100 --json tagName -q '.[].tagName')
if [ -z "$releases" ]; then
    echo "no releases published, nothing to compare against"
    exit 0
fi

status=0
for header in "${HEADERS[@]}"; do
    version=$(header_version "$header")
    if [ -z "$version" ]; then
        echo "FAIL  $header: no version banner found"
        echo "      expected a first banner line like: /* $header -- v1.2.3 -- ..."
        status=1
        continue
    fi

    strip_comments "$header" > "$work/local.c"

    clash=""
    for tag in $releases; do
        rm -f "$work/$header"
        gh release download "$tag" -p "$header" -D "$work" --clobber 2>/dev/null || continue

        released_version=$(header_version "$work/$header")
        [ "$released_version" = "$version" ] || continue

        strip_comments "$work/$header" > "$work/released.c"
        if ! diff -q "$work/local.c" "$work/released.c" >/dev/null; then
            clash="$tag"
            break
        fi
    done

    if [ -n "$clash" ]; then
        echo "FAIL  $header is $version, and release $clash publishes a different $version"
        echo "      A consumer cannot tell the two apart by reading the file."
        echo "      Bump the banner in $header and the version table in README.md."
        echo
        diff -u "$work/released.c" "$work/local.c" | head -40
        echo
        status=1
    else
        echo "ok    $header $version"
    fi
done

exit $status
