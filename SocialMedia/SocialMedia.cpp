// Social Media (Instagram)

// Requirements
   // User can register/login
   // Users can follow/unfollow each other using follow request (accept and decline feature). 
   // User can upload posts.
   // User can comment on each other posts. 
   // follow request notification, new post notification to followers. 
   // User can like/unlike other users posts. 
   // User can delete post
   // view feed.
   // public and private account
   // Block user.
   // view post from followed users. 
   // followers listing
   // following listing. 
   // save post/reel
   // user search functionality. 
   // User story feature


// Required Classes
   // User
   // Post
   // Comment
   // Notification
   // UserSearch
   // Story
   // Media 

   // SocialMediaService
   // IdGenerator

// Attributes/variable/data members
   // User : user_id, name, user_name, bio, profile_photo, 
   //        list of followers, list of following, block users, status. 

   // Post: post_id, user_id, list of Media, caption, song, status, created at, updated at, 
   //       list count, comment count

   // Comment : comment_id, post_id, user_id, like_count, parent_comment_id (-1 for root comment), status, date

   // Notification : notification_id, receiverUserId, actorUserId, action_type, notification_content, status (read, unread), date

   // Story : user_id, Media, list of views, like count, expires_at, date. 

   // Media : media_id, s3 url, type (image, video)





#include<iostream>
#include<string>
#include<vector>
#include<unordered_map>
#include<unordered_set>
#include<memory>
#include<algorithm>
#include<ctime>

using namespace std;

enum class MediaType { IMAGE, VIDEO };

enum class NotificationType { FOLLOW, LIKE, COMMENT };

enum class UserStatus { ACTIVE, INACTIVE, DELETED };

enum class PostStatus { ACTIVE, ARCHIVED, DELETED };

class IdGenerator {
  private: 
   int userId = 1;
   int postId = 1;
   int commentId = 1;
   int notificationId = 1;
   int storyId = 1;
   int mediaId = 1;

  public: 
   int generateUserId() {
      return userId++;
   }

   int generatePostId() {
      return postId++;
   }

   int generateCommentId() {
      return commentId++;
   }

   int generateNotificationId() {
      return notificationId++;
   }

   int generateStoryId() {
      return storyId++;
   }

   int generateMediaId() {
      return mediaId++;
   }

};

class Media {
   private: 
     int mediaId;
     MediaType type;
     string s3_url;

   public:
     Media(int mediaId, MediaType mediaType, string s3_url) : mediaId(mediaId) , type(mediaType), s3_url(s3_url) {}

     int getMediaId() {
      return mediaId;
     }

     MediaType getMediaType() {
      return type;
     }

     string getS3Url() {
      return s3_url;
     }
};

class User {
   private:
      int userId;
      string name;
      string userName;
      string bio;
      string profilePhotoUrl;
      unordered_set<int>followers;
      unordered_set<int>following;
      unordered_set<int>blockedUsers;
      UserStatus status;

   public:

      User(int userId, string name, string userName, string bio, string profilePhotoUrl)
       : userId(userId), name(name), userName(userName), bio(bio), profilePhotoUrl(profilePhotoUrl), status(UserStatus::ACTIVE) {}

      int getUserId() {
         return userId;
      }

      // other getter methods for name, userName, bio, profilePhototUrl

      void updateUserName(string newUserName) {
         userName = newUserName;
      }

      void addFollower(int userId) {
         followers.insert(userId);
      }

      void removeFollower(int userId) {
         followers.erase(userId);
      }

      void addFollowing(int userId) {
         following.insert(userId);
      }

      void removeFollowing(int userId) {
         following.erase(userId);
      }

      void blockUser(int userId) {
         blockedUsers.insert(userId);
      }

      void unBlockUser(int userId) {
         blockedUsers.erase(userId);
      }

      void updateUserStatus(UserStatus status) {
         status = status;
      }

      UserStatus getUserStatus() {
         return status;
      }
};

class Post {
   private: 
     int postId;
     int userId;
     vector<Media*>medias;
     string caption;
     string musicUrl;
     PostStatus status;
     int listCount;
     int commentCount;
     time_t createdAt;
     time_t updatedAt;
   
   public:
      int getPostId() {
         return postId;
      }

      
   
}