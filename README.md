![Backdrop CMS image](https://www.drupalforge.org/sites/default/files/styles/large/public/2024-06/thumbnail.jpg.webp)

# Backdrop CMS
Launch Backdrop CMS on Drupal Forge. If you launch with a cloud development environment, you can use it to contribute to Backdrop CMS Quickstart.

<div align="center">
   <a href="https://www.drupalforge.org/template/backdrop-cms">
      <figure>
         <img src="https://github.com/user-attachments/assets/69745ec7-d9a6-498f-9f47-8b60795195bb" height="100px" />
         <br />
         <figcaption>Launch Backdrop CMS</figcaption>
      </figure>
   </a>
</div>

## Contributing to Backdrop CMS Quickstart

1. [Fork](https://github.com/devpanel-demo/backdrop-cms-quickstart) the
Backdrop CMS Quickstart repository.

2. After launching Backdrop CMS, open DevPanel VS Code then open terminal and
   run the following commands:
   ```
   cd /var/www/html/
   git remote add <github_username> https://<github_username>:<github_personal_token>@github.com/<github_username>/<github_repo>
   ```
   It is best practice to generate a [GitHub personal access token](https://github.com/settings/tokens) rather than to use your GitHub password for `<github_personal_token>`. If you use a classic access token, it needs the `repo` permission. If you use a fine-grained access token, it needs read and write access to repository contents.

3. Create a branch for your issue:
   ```
   git checkout -b <issue-branch>
   ```

4. When you have committed your work, push the branch to your fork:
   ```
   git push -u <github_username> HEAD
   ```

5. Create a GitHub pull request from your fork back to the official [Backdrop CMS Quickstart](https://github.com/devpanel-demo/backdrop-cms-quickstart) repository.