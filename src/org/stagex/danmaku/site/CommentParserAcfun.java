package org.stagex.danmaku.site;

import java.io.FileInputStream;
import java.io.InputStream;
import java.math.BigInteger;
import java.util.ArrayList;
import java.util.HashMap;

import org.stagex.danmaku.comment.Comment;
import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserFactory;

public class CommentParserAcfun extends CommentParser {
	static String path = "http://124.228.254.234/newflvplayer/xmldata/"+"53388844"+"/comment_on.xml";
	static BigInteger R;
	static BigInteger G;
	static BigInteger B;
	@Override
	public ArrayList<Comment> parse(String uri) {
		ArrayList<Comment> result = new ArrayList<Comment>();
		try {
			if (uri.startsWith("file://")) {
				uri = uri.substring(7);
			} else {
				return null;
			}
			InputStream fin = new FileInputStream(uri);
			XmlPullParserFactory factory = XmlPullParserFactory.newInstance();
			XmlPullParser parser = factory.newPullParser();
			parser.setInput(fin, null);
			int commentTime = -1;
			int commentType = -1;
			int commentSize = -1;
			int commentColor = -1;
			String commentText = null;
			int currentDepth = 0;
			String tagName = null;
			for (int eventType = parser.getEventType(); eventType != XmlPullParser.END_DOCUMENT; eventType = parser
					.next()) {
				if (eventType == XmlPullParser.START_DOCUMENT) {
					continue;
				}
				if (eventType == XmlPullParser.START_TAG) {
					currentDepth += 1;
					tagName = parser.getName();
					if (currentDepth == 1
							&& tagName.compareTo("information") != 0) {
						break;
					}
					if (currentDepth == 3) {
						if (tagName.compareTo("message") == 0) {
							int count = parser.getAttributeCount();
							for (int i = 0; i < count; i++) {
								String name = parser.getAttributeName(i);
								String value = parser.getAttributeValue(i);
								if (name.compareTo("fontsize") == 0) {
									commentSize = Integer.parseInt(value);
									continue;
								}
								if (name.compareTo("color") == 0) {
									commentColor = Integer.parseInt(value);
									continue;
								}
								if (name.compareTo("mode") == 0) {
									commentType = Integer.parseInt(value);
									continue;
								}
							}
						}
					}
					continue;
				}
				if (eventType == XmlPullParser.END_TAG) {
					currentDepth -= 1;
					if (currentDepth == 1) {
						Comment comment = new Comment();
						comment.site = Comment.SITE_ACFUN;
						comment.time = commentTime;
						comment.type = commentType;
						comment.size = commentSize;
						comment.color = commentColor;
						comment.text = commentText;
						result.add(comment);
					}
					continue;
				}
				if (eventType == XmlPullParser.TEXT) {
					if (currentDepth == 3) {
						if (tagName.compareTo("playTime") == 0) {
							commentTime = (int) (Float.parseFloat(parser
									.getText()) * 1000);
						}
						if (tagName.compareTo("message") == 0) {
							commentText = parser.getText();
						}
					}
					continue;
				}
			}
			fin.close();
		} catch (Exception e) {

		}
		return result.size() > 0 ? result : null;
	}
	public static  ArrayList<HashMap> Pxml(String path){
		ArrayList<HashMap> list = new ArrayList<HashMap>();
		try {
			Connection c = Jsoup.connect(path);
			c.timeout(5000);
			Document doc;
			doc = c.get();
			Elements ems = doc.select("l");
			if(ems.size()!=0){
				for(Element em:ems){
					HashMap<String,Object> map = new HashMap<String, Object>();
					map.put("msg",em.text() );
					String[] attr = em.attr("i").split(",");
					map.put("time", attr[0]);
					map.put("size", attr[1]);
					map.put("color", attr[2]);
					map.put("mode", attr[3]);
					list.add(map);
				}
			}else{
					//use pull
			}
		} catch (IOException e) {
				// TODO Auto-generated catch block
			e.printStackTrace();
		}
		return list;
	}
	
	public void pRGB(String str){
		BigInteger src = new BigInteger(String.valueOf(65535),10);
		StringBuffer buffer = new StringBuffer();
		String s2 = src.toString(2);
		int len = s2.length();
		
		if(len<24){
			buffer.append(s2);
			int b = 24-len;
			for(int i=0;i<b;i++){
				buffer.append(0);
			}
			String s2p = buffer.toString();		
			R = new BigInteger(s2p.substring(0, 8),2);
			G = new BigInteger(s2p.substring(8, 16),2);
			B = new BigInteger(s2p.substring(16, s2p.length()),2);
			
		}else {
			R = new BigInteger(s2.substring(0, 8),2);
			G = new BigInteger(s2.substring(8, 16),2);
			B = new BigInteger(s2.substring(16, len),2);
			
		}
	}

}
