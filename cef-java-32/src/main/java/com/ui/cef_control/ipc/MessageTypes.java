package com.ui.cef_control.ipc;



//public final class MessageTypes {
//
//	public static final String HELLO = "HELLO";
//
//	public static final String OPEN_PAGE = "OPEN_PAGE";
//
//	public static final String PAGE_STATUS = "PAGE_STATUS";
//
//	private String MessageTypes() {
//		// Utility class - prevent instantiation
//
//
//    public static boolean isValid(String messageType) {
//        if (messageType == null) {
//            return false;
//        }
//        return HELLO.equals(messageType) || OPEN_PAGE.equals(messageType) || PAGE_STATUS.equals(messageType);
//    }
//
//	public static boolean isQueryType(String messageType) {
//		return PAGE_STATUS.equals(messageType);
//	}
//
//	public static boolean isCommandType(String messageType) {
//		return OPEN_PAGE.equals(messageType);
//	}
//
//
//	private static String from(String typeValue)
//	{
//		if (HELLO.equals(typeValue)) {
//			return HELLO;
//		} else if (OPEN_PAGE.equals(typeValue)) {
//			return OPEN_PAGE;
//		} else if (PAGE_STATUS.equals(typeValue)) {
//			return PAGE_STATUS;
//		} else {
//			throw new IllegalArgumentException("Unsupported message type: " + typeValue);
//		}
//	}
//}

public enum MessageTypes {
	HELLO,
	OPEN,
	NAVIGATE;

	public static MessageTypes from(String value) {
		for (MessageTypes type : values()) {
			if (type.name().equals(value)) {
				return type;
			}
		}
		throw new IllegalArgumentException("Unknown message type: " + value);
	}
}