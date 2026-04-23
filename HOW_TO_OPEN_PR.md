# How to open the PR

Run:

```bash
./fix_and_prepare_pr.sh
```

Then push the branch:

```bash
git push -u origin fix/loki97-stable-sidechannel
```

Finally open a Pull Request to `main` in GitHub UI.

## Quick validation command

```bash
./check_build.sh
```

If everything is healthy, it ends with:

`BUILD CHECK PASSED`
